#ifndef REPLACEMENT_ITP_H
#define REPLACEMENT_ITP_H

#include <cstdint>
#include <map>
#include <vector>

#include "cache.h"
#include "modules.h"

struct itp_set_helper {
    using rrpv_type = int32_t;
    static constexpr rrpv_type maxRRPV = 50; // ITP uses maxRRPV = 50

    std::vector<rrpv_type> rrpv_values;
    rrpv_type& get_rrpv(long way);

    explicit itp_set_helper(long ways);

    long victim();
    void update(long way, bool hit, bool is_instr, uint32_t pos);
    void reset(long way);
};

struct itp : public champsim::modules::replacement {
    uint32_t instr_pos, data_pos;
    uint32_t maxRRPV;

    std::vector<itp_set_helper> sets;
    std::map<uint64_t, int32_t> vpn_freq_acc;

    explicit itp(CACHE* cache);
    itp(CACHE* cache, long sets_, long ways_);

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
