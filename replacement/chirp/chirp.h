#ifndef REPLACEMENT_CHIRP_H
#define REPLACEMENT_CHIRP_H

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "cache.h"
#include "modules.h"
#include "msl/bits.h"

struct chirp : public champsim::modules::replacement
{
    // Configuration parameters
    static constexpr int threshold_bypass = 100;
    static constexpr int cache_thresh = 1;
    static constexpr int signature_bits = 16;
    static constexpr int pred_table_index_bits = 16;
    static constexpr int num_tables = 8;

    // Module type enumeration (from predTable.h)
    enum class cpu_structure {
        L1icache = 0,
        L1dcache = 5,
        L2cache = 6,
        L3cache = 7,
        BTB = 1,
        L1iTLB = 2,
        L1dTLB = 4,
        TLB2 = 3,
        indirBP = 9,
        BP = 8
    };

    // Sampler entry structure
    struct sampler_entry {
        unsigned int lru_stack_position = 0;
        unsigned int tag = 0;
        unsigned int trace = 0;
        int conf_val = 0;
        bool valid = false;
        bool prediction = false;
    };

    struct sampler_set {
        std::vector<sampler_entry> blocks;
    };

    struct predTable {
        int counter_max;
        int counter_min, counter_width = 2;
        int predictor_index_bits, predictor_table_entries, predictor_tables;
        std::vector<std::vector<int>> tables;

        predTable(int pred_inx_bits, int pred_num_tables);
        uint64_t get_table_index(int type, uint64_t trace, int t);
        int get_prediction(int type, uint64_t trace);
        void block_is_dead(int type, uint64_t trace, bool d);
    };

    // Runtime configuration (initialized in constructor)
    uint32_t _sampler_set;
    uint32_t _sampler_assoc;
    uint32_t sampler_index_offset;
    uint32_t sampler_blk_offset;
    uint32_t sampler_nblcks;
    uint32_t _sampler_setsize;
    uint32_t sampler_modulus;

    // Statistics
    int nvict = 0;
    int bypass_cnt = 0;
    int samp_dead_victim_cnt = 0;
    int cache_dead_victim_cnt = 0;
    int matched_samp_cnt = 0;
    int empty_sampler_cnt = 0;
    int lru_sampler_cnt = 0;
    int empty_cache_cnt = 0;
    int cache_samp_dead_cnt = 0;
    uint64_t nfalseneg = 0;
    uint64_t nfalsepos = 0;
    uint64_t deadlru = 0;
    uint64_t nevict_samp = 0;
    int cache_non_samp_dead_cnt = 0;

    // Branch history (to be provided by the CPU)
    uint64_t condHistory_old = 0;
    uint64_t uncondIndHistory_old = 0;
    uint64_t global_path_history_MHRP = 0;

    // Internal state
    cpu_structure module_type;
    std::vector<uint64_t> last_used_cycles;
    std::vector<bool> is_dead;
    std::vector<sampler_set> sampler_sets;
    predTable _predTable;
    uint32_t last_set;

    explicit chirp(CACHE* cache);
    chirp(CACHE* cache, long sets_, long ways_);

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

    // Helper functions
    uint64_t make_signature(uint64_t pc, const std::string& name);
    uint64_t make_signature(uint64_t pc, const std::string& name, uint64_t cond_hist, uint64_t uncond_hist, uint64_t path_hist);
    void set_branch_history(uint64_t cond, uint64_t uncond, uint64_t global);
};

// External flag for prediction table update control
extern bool table_update_flag;

#endif // REPLACEMENT_CHIRP_H