#ifndef REPLACEMENT_XPTP_H
#define REPLACEMENT_XPTP_H

#include <array>
#include <vector>

#include "cache.h"
#include "modules.h"

/*
 * Note: This is only for cache use (no tlb)
 */

struct xptp : public champsim::modules::replacement {
private:
    typedef struct _eviction_entry {
        uint32_t lru_value;
        bool is_pte;
        bool is_data;

        _eviction_entry() : lru_value(0), is_pte(false), is_data(false) {}
    } eviction_entry;

    double TLB_LOWER_STRESS_THRESHOLD;
    double TLB_UPPER_STRESS_THRESHOLD;
    std::vector<eviction_entry> least_recently_used;
    uint32_t MIN_EVICTION_POSITION;

public:
    explicit xptp(CACHE* cache);
    xptp(CACHE* cache, long sets_, long ways_);

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