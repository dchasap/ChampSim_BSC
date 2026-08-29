#include "tship.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <map>
#include <utility>
#include <vector>

#include "cache.h"
#include "champsim.h"
#include "msl/bits.h"

// T-SHIP requires the EXPAND_PACKET macro to be defined so that is_pte, is_replay, and
// translation_level are available to the replacement policy. Without it, the policy
// cannot correctly handle translation accesses, so we refuse to compile.
#if !defined(EXPAND_PACKET)
#error "T-SHIP replacement policy requires the build to define EXPAND_PACKET. \
Add -DEXPAND_PACKET to your CXXFLAGS or build configuration."
#endif

namespace
{
constexpr int maxRRPV = 3;
constexpr std::size_t SHCT_SIZE = 16384;
constexpr unsigned SHCT_PRIME = 16381;
constexpr unsigned SHCT_MAX = 7;

// sampler
std::map<CACHE*, std::vector<std::size_t>> rand_sets;
std::map<CACHE*, std::vector<tship::SAMPLER_class>> sampler;
std::map<CACHE*, std::vector<int>> rrpv_values;

// prediction table structure
std::map<std::pair<CACHE*, std::size_t>, std::array<unsigned, SHCT_SIZE>> SHCT;

std::size_t get_sampler_set_count()
{
    return 256 * NUM_CPUS;
}
} // namespace

tship::tship(CACHE* cache) : tship(cache, cache->NUM_SET, cache->NUM_WAY) {}

tship::tship(CACHE* cache, long sets_, long ways_) : replacement(cache)
{
    (void)sets_;
    (void)ways_;
    SAMPLER_SET = get_sampler_set_count();
}

void tship::initialize_replacement()
{
    // randomly selected sampler sets
    std::size_t rand_seed = 1103515245 + 12345;
    for (std::size_t i = 0; i < static_cast<std::size_t>(SAMPLER_SET); i++) {
        std::size_t val = (rand_seed / 65536) % static_cast<std::size_t>(intern_->NUM_SET);
        auto loc = std::lower_bound(std::begin(::rand_sets[intern_]), std::end(::rand_sets[intern_]), val);

        while (loc != std::end(::rand_sets[intern_]) && *loc == val) {
            rand_seed = rand_seed * 1103515245 + 12345;
            val = (rand_seed / 65536) % static_cast<std::size_t>(intern_->NUM_SET);
            loc = std::lower_bound(std::begin(::rand_sets[intern_]), std::end(::rand_sets[intern_]), val);
        }

        ::rand_sets[intern_].insert(loc, val);
    }

    ::sampler[intern_] = std::vector<SAMPLER_class>(SAMPLER_SET * static_cast<std::size_t>(intern_->NUM_WAY));
    ::rrpv_values[intern_] = std::vector<int>(static_cast<std::size_t>(intern_->NUM_SET * intern_->NUM_WAY), maxRRPV);
}

// find replacement victim
long tship::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                        champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                        ,
                        CACHE::repl_pol_xargs xargs
#endif
)
{
    (void)triggering_cpu;
    (void)instr_id;
    (void)current_set;
    (void)ip;
    (void)full_addr;
    (void)type;
#if defined(EXPAND_PACKET)
    (void)xargs;
#endif

    // look for the maxRRPV line
    auto begin = std::next(std::begin(::rrpv_values[intern_]), set * intern_->NUM_WAY);
    auto end = std::next(begin, intern_->NUM_WAY);
    auto victim = std::find(begin, end, maxRRPV);
    while (victim == end) {
        for (auto it = begin; it != end; ++it)
            ++(*it);

        victim = std::find(begin, end, maxRRPV);
    }

    assert(begin <= victim);
    return static_cast<long>(std::distance(begin, victim));
}

// called on every cache hit and cache fill
void tship::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                     champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                     ,
                                     CACHE::repl_pol_xargs xargs
#endif
)
{
    // handle writeback access
    if (type == access_type::WRITE) {
        if (!hit)
            ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = maxRRPV - 1;

        return;
    }

    // update sampler
    auto s_idx = std::find(std::begin(::rand_sets[intern_]), std::end(::rand_sets[intern_]), static_cast<std::size_t>(set));
    if (s_idx != std::end(::rand_sets[intern_])) {
        auto s_set_begin = std::next(std::begin(::sampler[intern_]), std::distance(std::begin(::rand_sets[intern_]), s_idx));
        auto s_set_end = std::next(s_set_begin, intern_->NUM_WAY);

        // check hit
        const std::size_t way_bits = champsim::lg2(intern_->NUM_WAY);
        const auto addr_tag = full_addr.to<uint64_t>() >> way_bits;
        decltype(s_set_begin) match;
        for (match = s_set_begin; match != s_set_end; ++match) {
            if (match->valid && ((match->address.to<uint64_t>()) >> way_bits) == addr_tag)
                break;
        }
        if (match != s_set_end) {
#if defined(EXPAND_PACKET)
            // T-SHIP addition, modify signature accordingly for replay and translations loads
            std::size_t SHCT_idx = ip.to<std::size_t>() << (xargs.is_pte + xargs.is_replay);
            SHCT_idx = SHCT_idx % ::SHCT_PRIME;
            if (::SHCT[std::make_pair(intern_, triggering_cpu)][SHCT_idx] > 0)
                ::SHCT[std::make_pair(intern_, triggering_cpu)][SHCT_idx]--;
#endif

            match->used = 1;
        } else {
            match = std::min_element(s_set_begin, s_set_end, [](auto x, auto y) { return x.last_used < y.last_used; });

            if (match->used) {
#if defined(EXPAND_PACKET)
                // T-SHIP addition, modify signature accordingly for replay and translations loads
                std::size_t SHCT_idx = ip.to<std::size_t>() << (xargs.is_pte + xargs.is_replay);
                SHCT_idx = SHCT_idx % ::SHCT_PRIME;
                if (::SHCT[std::make_pair(intern_, triggering_cpu)][SHCT_idx] < ::SHCT_MAX)
                    ::SHCT[std::make_pair(intern_, triggering_cpu)][SHCT_idx]++;
#endif
            }

            match->valid = 1;
            match->address = full_addr;
            match->ip = ip;
            match->used = 0;
        }

        // update LRU state using access_count
        match->last_used = access_count++;
    }

    if (hit)
        ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = 0;
    else {
        // SHIP prediction
#if defined(EXPAND_PACKET)
        // T-SHIP addition, modify signature accordingly for replay and translations loads
        std::size_t SHCT_idx = ip.to<std::size_t>() << (xargs.is_pte + xargs.is_replay);
        SHCT_idx = SHCT_idx % ::SHCT_PRIME;

        ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = maxRRPV - 1;
        if (::SHCT[std::make_pair(intern_, triggering_cpu)][SHCT_idx] == ::SHCT_MAX)
            ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = maxRRPV;
#else
        ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = maxRRPV - 1;
#endif
    }

#if defined(EXPAND_PACKET)
    if (xargs.is_pte && xargs.translation_level == 0) {
        ::rrpv_values[intern_][set * intern_->NUM_WAY + way] = 0;
        return;
    }
#else
    (void)victim_addr;
#endif
}

void tship::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                   ,
                                   CACHE::repl_pol_xargs xargs
#endif
)
{
    // T-SHIP has no special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void tship::replacement_final_stats()
{
    // No additional stats to print
}
