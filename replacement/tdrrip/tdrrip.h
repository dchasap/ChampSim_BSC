#ifndef REPLACEMENT_TDRIP_H
#define REPLACEMENT_TDRIP_H

#include <map>
#include <vector>
#include <algorithm>

#include "cache.h"
#include "modules.h"
#include "msl/fwcounter.h"

namespace champsim
{
    extern const std::size_t NUM_CPUS;
}

struct tdrrip : public champsim::modules::replacement
{
    explicit tdrrip(CACHE* cache);
    tdrrip(CACHE* cache, long sets_, long ways_);

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

#endif // REPLACEMENT_TDRIP_H