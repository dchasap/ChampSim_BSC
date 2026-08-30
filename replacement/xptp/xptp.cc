#include "xptp.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "cache.h"
#include "env_var.h"

/*
 * Note: This is only for cache use (no tlb)
 */

// XPTP requires the EXPAND_PACKET macro to be defined so that is_instr and is_pte
// are available to the replacement policy. Without it, the policy cannot correctly
// distinguish instruction/PTE accesses, so we refuse to compile.
#if !defined(EXPAND_PACKET)
#error "XPTP replacement policy requires the build to define EXPAND_PACKET. \
Add -DEXPAND_PACKET to your CXXFLAGS or build configuration."
#endif

// Global STLB MPKI - updated by CACHE code in src/cache.cc whenever STLB misses occur.
// This matches the ChampSim_old behavior where STLB_MPKI was computed from TLB misses
// and used by xPTP to gate its stress-aware eviction policy.
double STLB_MPKI = 0.0;

xptp::xptp(CACHE* cache) : xptp(cache, cache->NUM_SET, cache->NUM_WAY) {}

xptp::xptp(CACHE* cache, long sets_, long ways_) : replacement(cache) {
    (void)sets_;
    (void)ways_;
    MIN_EVICTION_POSITION = 0;
    TLB_LOWER_STRESS_THRESHOLD = 0.0;
    TLB_UPPER_STRESS_THRESHOLD = 0.0;
}

void xptp::initialize_replacement() {
    if (auto v = champsim::EnvVar<int>::get("TLB_LOWER_STRESS_THRESHOLD")) {
        TLB_LOWER_STRESS_THRESHOLD = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("TLB_UPPER_STRESS_THRESHOLD")) {
        TLB_UPPER_STRESS_THRESHOLD = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("MIN_EVICTION_POSITION")) {
        MIN_EVICTION_POSITION = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("MIN_EVICTION_POSITION_L1D")) {
        if (intern_->NAME.compare("cpu0_L1D") == 0)
            MIN_EVICTION_POSITION = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("MIN_EVICTION_POSITION_L2C")) {
        if (intern_->NAME.compare("cpu0_L2C") == 0)
            MIN_EVICTION_POSITION = *v;
    }

    std::cout << intern_->NAME << " is using xPTP" << std::endl;
    std::cout << "\tTLB_LOWER_STRESS_THRESHOLD:" << TLB_LOWER_STRESS_THRESHOLD << std::endl;
    std::cout << "\tTLB_UPPER_STRESS_THRESHOLD:" << TLB_UPPER_STRESS_THRESHOLD << std::endl;
    std::cout << "\tMIN_EVICTION_POSITION:" << MIN_EVICTION_POSITION << std::endl;

    least_recently_used = std::vector<eviction_entry>(intern_->NUM_SET * intern_->NUM_WAY);
}

// find replacement victim
long xptp::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                        const champsim::cache_block* current_set, champsim::address ip,
                        champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                        ,
                        CACHE::repl_pol_xargs xargs
#endif
                        ) {
    (void)triggering_cpu;
    (void)instr_id;
    (void)ip;
    (void)full_addr;
    (void)type;
#if defined(EXPAND_PACKET)
    (void)xargs;
#endif

    // first lookup for an invalid entry
    for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
        if (!current_set[i].valid) {
            return i;
        }
    }

    bool found_data_candidate = false;
    uint32_t lru_victim_index = 0;
    uint32_t alt_victim_index = 0, max_alt_lru = 0;
    for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
        if (least_recently_used[set * intern_->NUM_WAY + i].lru_value == (intern_->NUM_WAY - 1)) {
            lru_victim_index = i;
        }

        if (!(least_recently_used[set * intern_->NUM_WAY + i].is_pte)
                && least_recently_used[set * intern_->NUM_WAY + i].lru_value > max_alt_lru) {

            found_data_candidate = true;
            max_alt_lru = least_recently_used[set * intern_->NUM_WAY + i].lru_value;
            alt_victim_index = i;
        }
    }

    if (found_data_candidate && max_alt_lru >= MIN_EVICTION_POSITION
            && ((STLB_MPKI >= TLB_LOWER_STRESS_THRESHOLD) && (STLB_MPKI <= TLB_UPPER_STRESS_THRESHOLD))) {

        // adjust lru value to entries that should have been evicted instead
        for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
            if (least_recently_used[set * intern_->NUM_WAY + i].lru_value >= max_alt_lru) {
                least_recently_used[set * intern_->NUM_WAY + i].lru_value--;
            }
        }
        return alt_victim_index;
    }

    return lru_victim_index;
}

// called on every cache hit and cache fill
void xptp::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                     champsim::address full_addr, champsim::address ip,
                                     champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                     ,
                                     CACHE::repl_pol_xargs xargs
#endif
                                     ) {
    (void)triggering_cpu;
    (void)full_addr;
    (void)ip;
    (void)victim_addr;
    (void)type;
    (void)hit;

    uint32_t hit_lru = least_recently_used[set * intern_->NUM_WAY + way].lru_value;

    for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
        if (least_recently_used[set * intern_->NUM_WAY + i].lru_value <= hit_lru) {
            least_recently_used[set * intern_->NUM_WAY + i].lru_value++;
        }
    }
    least_recently_used[set * intern_->NUM_WAY + way].lru_value = 0; // promote to the MRU position

#if defined(EXPAND_PACKET)
    // Use xargs.is_instr to determine is_data (instr accesses are not data)
    // Use xargs.is_pte to set the is_pte flag
    least_recently_used[set * intern_->NUM_WAY + way].is_data = !xargs.is_instr;
    least_recently_used[set * intern_->NUM_WAY + way].is_pte = xargs.is_pte;
#else
    least_recently_used[set * intern_->NUM_WAY + way].is_data = !(type == access_type::LOAD);
    least_recently_used[set * intern_->NUM_WAY + way].is_pte = false;
#endif
}

void xptp::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                    champsim::address full_addr, champsim::address ip,
                                    champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                    ,
                                    CACHE::repl_pol_xargs xargs
#endif
                                    ) {
    // xptp doesn't have special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void xptp::replacement_final_stats() {
    // No additional stats to print
}