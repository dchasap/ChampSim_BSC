#ifndef REPLACEMENT_PTP_H
#define REPLACEMENT_PTP_H

#include <vector>

#include "cache.h"
#include "modules.h"

/*
 * Note: PTP - Page Table Priority Replacement Policy
 * Based on the paper "Every Walk is Hit"
 * 
 * This replacement policy prioritizes evicting data entries over PTE entries
 * when the PTE eviction ratio is below a configurable threshold.
 */

struct ptp : public champsim::modules::replacement {
private:
    typedef struct _eviction_entry {
        uint32_t lru_value;
        bool is_pte;

        _eviction_entry() : lru_value(0), is_pte(false) {}
    } eviction_entry;

    double TLB_STRESS_THRESHOLD;
    double PTE_EVICTION_RATIO;
    std::vector<eviction_entry> least_recently_used;
    
    // Statistics
    double current_pte_eviction_ratio;
    uint32_t total_pte_evictions;
    uint32_t total_evictions;
    uint32_t current_tlb_stress_threshold;

public:
    explicit ptp(CACHE* cache);
    ptp(CACHE* cache, long sets_, long ways_);

    void initialize_replacement();
    long find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                    champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                    ,
                    CACHE::repl_pol_xargs xargs
#endif
                    );
    void update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                ,
                                CACHE::repl_pol_xargs xargs
#endif
                                );
    void replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                               champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                               ,
                               CACHE::repl_pol_xargs xargs
#endif
                               );
    void replacement_final_stats();
};

#endif
