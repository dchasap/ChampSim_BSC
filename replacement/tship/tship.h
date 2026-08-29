#ifndef REPLACEMENT_TSHIP_H
#define REPLACEMENT_TSHIP_H

#include <array>
#include <map>
#include <vector>
#include <algorithm>

#include "cache.h"
#include "modules.h"
#include "msl/bits.h"

struct tship : public champsim::modules::replacement
{
    static constexpr int maxRRPV = 3;
    static constexpr std::size_t SHCT_SIZE = 16384;
    static constexpr unsigned SHCT_PRIME = 16381;
    static constexpr unsigned SHCT_MAX = 7;

    long NUM_SET, NUM_WAY;
    long SAMPLER_SET;
    uint64_t access_count = 0;

    // sampler structure
    class SAMPLER_class
    {
    public:
        bool valid = false;
        uint8_t used = 0;
        champsim::address address{};
        champsim::address cl_addr{};
        champsim::address ip{};
        uint64_t last_used = 0;
    };

    // sampler
    std::map<CACHE*, std::vector<std::size_t>> rand_sets;
    std::vector<SAMPLER_class> sampler;
    std::vector<int> rrpv_values;

    // prediction table structure
    std::map<std::pair<CACHE*, std::size_t>, std::array<unsigned, SHCT_SIZE>> SHCT;

    explicit tship(CACHE* cache);
    tship(CACHE* cache, long sets_, long ways_);

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

#endif // REPLACEMENT_TSHIP_H