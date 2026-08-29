#include "ptp.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "cache.h"
#include "env_var.h"

/*
 * Note: Hardware Implementation of paper:
 * Every Walk is Hit
 */

//#define MIN_EVICTION_POSITION 6

// PTP requires the EXPAND_PACKET macro to be defined so that is_pte
// is available to the replacement policy. Without it, the policy cannot
// correctly distinguish PTE accesses, so we refuse to compile.
#if !defined(EXPAND_PACKET)
#error "PTP replacement policy requires the build to define EXPAND_PACKET. \
Add -DEXPAND_PACKET to your CXXFLAGS or build configuration."
#endif

ptp::ptp(CACHE* cache) : ptp(cache, cache->NUM_SET, cache->NUM_WAY) {}

ptp::ptp(CACHE* cache, long sets_, long ways_) : replacement(cache) {
    (void)sets_;
    (void)ways_;
    TLB_STRESS_THRESHOLD = 0.0;
    PTE_EVICTION_RATIO = 0.0;
    current_pte_eviction_ratio = 0.0;
    total_pte_evictions = 0;
    total_evictions = 0;
    current_tlb_stress_threshold = 0;
}

void ptp::initialize_replacement() {
    if (auto v = champsim::EnvVar<int>::get("TLB_STRESS_THRESHOLD")) {
        TLB_STRESS_THRESHOLD = *v;
    }

    if (auto v = champsim::EnvVar<double>::get("PTE_EVICTION_RATIO")) {
        PTE_EVICTION_RATIO = *v;
    }

    std::cout << intern_->NAME << " is using PTP replacement policy:" << std::endl;
    std::cout << "\tTLB_STRESS_THRESHOLD:" << TLB_STRESS_THRESHOLD << std::endl;
    std::cout << "\tPTE_EVICTION_RATIO:" << PTE_EVICTION_RATIO << std::endl;

    current_pte_eviction_ratio = 0.0;
    total_pte_evictions = 0;
    total_evictions = 0;
    current_tlb_stress_threshold = 0;
    least_recently_used = std::vector<eviction_entry>(intern_->NUM_SET * intern_->NUM_WAY);
}

// find replacement victim
long ptp::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
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

    total_evictions++;

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

    // Compute current pte eviction ratio
    double pte_eviction_ratio = static_cast<double>(total_pte_evictions) / static_cast<double>(total_evictions);
    current_pte_eviction_ratio = std::round((100 * pte_eviction_ratio) * 0.5f) * 2;

    if (found_data_candidate
            //&& (vmem->STLB_MISS_RATE >= TLB_STRESS_THRESHOLD)
            && (current_pte_eviction_ratio <= PTE_EVICTION_RATIO)) {
        // adjust lru value to entries that should have been evicted instead
        for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
            if (least_recently_used[set * intern_->NUM_WAY + i].lru_value >= max_alt_lru) {
                least_recently_used[set * intern_->NUM_WAY + i].lru_value--;
            }
        }

        return alt_victim_index;
    }

    if (least_recently_used[set * intern_->NUM_WAY + lru_victim_index].is_pte)
        total_pte_evictions++;

    return lru_victim_index;
}

// called on every cache hit and cache fill
void ptp::update_replacement_state(uint32_t triggering_cpu, long set, long way,
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
    least_recently_used[set * intern_->NUM_WAY + way].is_pte = xargs.is_pte;
#else
    least_recently_used[set * intern_->NUM_WAY + way].is_pte = false;
#endif
}

void ptp::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                   champsim::address full_addr, champsim::address ip,
                                   champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                   ,
                                   CACHE::repl_pol_xargs xargs
#endif
                                   ) {
    // PTP doesn't have special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void ptp::replacement_final_stats() {
    // No additional stats to print
}
