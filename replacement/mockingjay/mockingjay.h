#ifndef REPLACEMENT_MOCKINGJAY_H
#define REPLACEMENT_MOCKINGJAY_H

#include <unordered_map>
#include <vector>

#include "cache.h"
#include "modules.h"
#include "msl/bits.h"

struct mockingjay : public champsim::modules::replacement
{
    static constexpr int maxRRPV = 3;

    long NUM_SET, NUM_WAY;
    int LOG2_NUM_SET;
    int LOG2_LLC_SIZE;
    int LOG2_SAMPLED_SETS;
    int HISTORY;
    int GRANULARITY;
    int INF_RD;
    int INF_ETR;
    int MAX_RD;

    int SAMPLED_CACHE_WAYS;
    int LOG2_SAMPLED_CACHE_SETS;
    int SAMPLED_CACHE_TAG_BITS;
    int PC_SIGNATURE_BITS;
    int TIMESTAMP_BITS;

    double TEMP_DIFFERENCE;
    double FLEXMIN_PENALTY;

    // ETR (Eviction Table Record) and timestamps
    std::vector<std::vector<int>> etr;
    std::vector<int> etr_clock;
    std::vector<int> current_timestamp;

    // PC signatures and RDP (Recent PC Directory)
    std::unordered_map<uint32_t, int> rdp;

    // Sampled cache structure
    struct SampledCacheLine
    {
        bool valid;
        uint64_t tag;
        uint64_t signature;
        int timestamp;
    };
    std::unordered_map<uint32_t, std::vector<SampledCacheLine*>> sampled_cache;

    explicit mockingjay(CACHE* cache);
    mockingjay(CACHE* cache, long sets_, long ways_);

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

#endif // REPLACEMENT_MOCKINGJAY_H