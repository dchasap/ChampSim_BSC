#include "tdrrip.h"

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
#include "msl/fwcounter.h"

// TDRRIP requires the EXPAND_PACKET macro to be defined so that is_pte, is_replay, and
// translation_level are available to the replacement policy. Without it, the policy
// cannot correctly handle translation accesses, so we refuse to compile.
#if !defined(EXPAND_PACKET)
#error "TDRRIP replacement policy requires the build to define EXPAND_PACKET. \
Add -DEXPAND_PACKET to your CXXFLAGS or build configuration."
#endif

namespace
{
constexpr unsigned maxRRPV = 3;
constexpr std::size_t NUM_POLICY = 2;
constexpr std::size_t SDM_SIZE = 32;
constexpr unsigned BIP_MAX = 32;
constexpr unsigned PSEL_WIDTH = 10;

std::map<CACHE*, unsigned> bip_counter;
std::map<CACHE*, std::vector<std::size_t>> rand_sets;
std::map<std::pair<CACHE*, std::size_t>, champsim::msl::fwcounter<PSEL_WIDTH>> PSEL;
std::map<CACHE*, std::vector<unsigned>> rrpv;

std::size_t total_sdm_sets()
{
    return NUM_CPUS * NUM_POLICY * SDM_SIZE;
}
} // namespace

tdrrip::tdrrip(CACHE* cache) : tdrrip(cache, cache->NUM_SET, cache->NUM_WAY) {}

tdrrip::tdrrip(CACHE* cache, long sets_, long ways_) : replacement(cache)
{
    (void)sets_;
    (void)ways_;
}

void tdrrip::initialize_replacement()
{
    // randomly selected sampler sets
    std::size_t rand_seed = 1103515245 + 12345;
    for (std::size_t i = 0; i < ::total_sdm_sets(); i++) {
        std::size_t val = (rand_seed / 65536) % static_cast<std::size_t>(intern_->NUM_SET);
        auto loc = std::lower_bound(std::begin(::rand_sets[intern_]), std::end(::rand_sets[intern_]), val);

        while (loc != std::end(::rand_sets[intern_]) && *loc == val) {
            rand_seed = rand_seed * 1103515245 + 12345;
            val = (rand_seed / 65536) % static_cast<std::size_t>(intern_->NUM_SET);
            loc = std::lower_bound(std::begin(::rand_sets[intern_]), std::end(::rand_sets[intern_]), val);
        }

        ::rand_sets[intern_].insert(loc, val);
    }

    ::rrpv[intern_] = std::vector<unsigned>(static_cast<std::size_t>(intern_->NUM_SET * intern_->NUM_WAY), 0);
}

// find replacement victim
long tdrrip::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
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
    auto begin = std::next(std::begin(::rrpv[intern_]), set * intern_->NUM_WAY);
    auto end = std::next(begin, intern_->NUM_WAY);

    auto victim = std::max_element(begin, end);
    for (auto it = begin; it != end; ++it)
        *it += ::maxRRPV - *victim;

    assert(begin <= victim);
    assert(victim < end);
    return static_cast<long>(std::distance(begin, victim));
}

// called on every cache hit and cache fill
void tdrrip::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                      champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                      ,
                                      CACHE::repl_pol_xargs xargs
#endif
)
{
    (void)victim_addr;

    // do not update replacement state for writebacks
    if (type == access_type::WRITE) {
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV - 1;
        return;
    }

    // cache hit
    if (hit) {
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = 0; // for cache hit, DRRIP always promotes a cache line to the MRU position
        return;
    }

    // cache miss
    auto begin = std::next(std::begin(::rand_sets[intern_]), triggering_cpu * ::NUM_POLICY * ::SDM_SIZE);
    auto end = std::next(begin, ::NUM_POLICY * ::SDM_SIZE);
    auto leader = std::find(begin, end, static_cast<std::size_t>(set));

    if (leader == end) { // follower sets
        auto selector = ::PSEL[std::make_pair(intern_, triggering_cpu)];
        if (selector.value() > (selector.maximum / 2)) { // follow BIP
            ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV;

            ::bip_counter[intern_]++;
            if (::bip_counter[intern_] == ::BIP_MAX) {
                ::bip_counter[intern_] = 0;
                ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV - 1;
            }
        } else { // follow SRRIP
            ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV - 1;
        }
    } else if (leader == begin) { // leader 0: BIP
        ::PSEL[std::make_pair(intern_, triggering_cpu)]--;
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV;

        ::bip_counter[intern_]++;
        if (::bip_counter[intern_] == ::BIP_MAX) {
            ::bip_counter[intern_] = 0;
            ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV - 1;
        }
    } else if (leader == std::next(begin)) { // leader 1: SRRIP
        ::PSEL[std::make_pair(intern_, triggering_cpu)]++;
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV - 1;
    }

#if defined(EXPAND_PACKET)
    if (xargs.is_pte && xargs.translation_level == 0) {
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = 0;
        return;
    }

    if (xargs.is_replay) {
        ::rrpv[intern_][set * intern_->NUM_WAY + way] = ::maxRRPV;
        return;
    }
#else
    (void)full_addr;
    (void)ip;
#endif
}

void tdrrip::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                    champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                    ,
                                    CACHE::repl_pol_xargs xargs
#endif
)
{
    // TDRRIP has no special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void tdrrip::replacement_final_stats()
{
    // No additional stats to print
}
