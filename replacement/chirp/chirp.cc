#include "chirp.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string.h>

#include "cache.h"
#include "champsim.h"

// External flag for prediction table update control
bool table_update_flag = false;

namespace
{
// Hash function mix from predTable.h
uint64_t mix(uint64_t a, uint64_t b, uint64_t c)
{
    a -= b;
    a -= c;
    a ^= (c >> 13);
    b -= c;
    b -= a;
    b ^= (a << 8);
    c -= a;
    c -= b;
    c ^= (b >> 13);
    a -= b;
    a -= c;
    a ^= (c >> 12);
    b -= c;
    b -= a;
    b ^= (a << 16);
    c -= a;
    c -= b;
    c ^= (b >> 5);
    a -= b;
    a -= c;
    a ^= (c >> 3);
    b -= c;
    b -= a;
    b ^= (a << 10);
    c -= a;
    c -= b;
    c ^= (b >> 15);
    return c;
}

uint64_t f1(uint64_t x)
{
    uint64_t fone = mix(0xfeedface, 0xdeadb10c, x);
    return fone;
}

uint64_t f2(uint64_t x)
{
    uint64_t ftwo = mix(0xc001d00d, 0xfade2b1c, x);
    return ftwo;
}

uint64_t fi(uint64_t x, int i)
{
    uint64_t ind = (f1(x)) + (f2(x));
    return ind;
}
} // namespace

// predTable implementation
chirp::predTable::predTable(int pred_inx_bits, int pred_num_tables)
{
    predictor_tables = pred_num_tables;
    predictor_index_bits = pred_inx_bits;
    predictor_table_entries = (1 << predictor_index_bits);
    counter_max = (1 << counter_width) - 1;
    counter_min = -counter_max;

    tables.resize(predictor_tables);
    for (int i = 0; i < predictor_tables; i++) {
        tables[i].resize(predictor_table_entries, 0);
    }
}

uint64_t chirp::predTable::get_table_index(int type, uint64_t trace, int t)
{
    uint64_t x1 = fi(trace, t);
    uint64_t x2 = x1 & ((1 << predictor_index_bits) - 1);
    return x2;
}

int chirp::predTable::get_prediction(int type, uint64_t trace)
{
    int conf = 0;
    int i = type;
    uint64_t index = get_table_index(type, trace, i);
    conf += tables[i][index];
    return conf;
}

void chirp::predTable::block_is_dead(int type, uint64_t trace, bool d)
{
    int* c = &tables[type][get_table_index(type, trace, type)];
    if (d == true) {
        if (*c < counter_max)
            (*c)++;
    } else {
        if (*c > counter_min && table_update_flag == true) {
            (*c)--;
        }
    }
}

// chirp implementation
chirp::chirp(CACHE* cache) : chirp(cache, cache->NUM_SET, cache->NUM_WAY) {}

chirp::chirp(CACHE* cache, long sets_, long ways_) : replacement(cache), _predTable(pred_table_index_bits, num_tables)
{
    (void)sets_;
    (void)ways_;
}

void chirp::initialize_replacement()
{
    _sampler_set = intern_->NUM_SET;
    _sampler_assoc = intern_->NUM_WAY;
    sampler_index_offset = champsim::msl::lg2(_sampler_set);
    sampler_blk_offset = champsim::msl::lg2(PAGE_SIZE);
    sampler_nblcks = (_sampler_set * _sampler_assoc) / PAGE_SIZE;
    _sampler_setsize = _sampler_assoc * PAGE_SIZE;

    // init module_type (only for TLB and caches)
    if (intern_->NAME.compare("cpu0_ITLB") == 0) {
        module_type = cpu_structure::L1iTLB;
    } else if (intern_->NAME.compare("cpu0_DTLB") == 0) {
        module_type = cpu_structure::L1dTLB;
    } else if (intern_->NAME.compare("cpu0_STLB") == 0) {
        module_type = cpu_structure::TLB2;
    } else if (intern_->NAME.compare("cpu0_L1I") == 0) {
        module_type = cpu_structure::L1icache;
    } else if (intern_->NAME.compare("cpu0_L1D") == 0) {
        module_type = cpu_structure::L1dcache;
    } else if (intern_->NAME.compare("cpu0_L2C") == 0) {
        module_type = cpu_structure::L2cache;
    } else if (intern_->NAME.compare("LLC") == 0) {
        module_type = cpu_structure::L3cache;
    }

    // init last_used_cycles for LRU
    last_used_cycles = std::vector<uint64_t>(intern_->NUM_SET * intern_->NUM_WAY, 0);
    // init is_dead for CHiRP predictions
    is_dead = std::vector<bool>(intern_->NUM_SET * intern_->NUM_WAY, false);

    // init sampler sets
    uint32_t nsampler_sets = _sampler_set;
    sampler_modulus = _sampler_set / nsampler_sets;
    sampler_sets.resize(nsampler_sets);
    for (auto& set : sampler_sets) {
        set.blocks.resize(_sampler_assoc);
        for (std::size_t i = 0; i < _sampler_assoc; ++i) {
            set.blocks[i].lru_stack_position = static_cast<uint32_t>(i);
            set.blocks[i].conf_val = 0;
            set.blocks[i].valid = false;
            set.blocks[i].tag = 0;
            set.blocks[i].trace = 0;
            set.blocks[i].prediction = false;
        }
    }

    std::cout << intern_->NAME << " is using CHiRP" << std::endl;
}

uint64_t chirp::make_signature(uint64_t pc, const std::string& name)
{
    return make_signature(pc, name, condHistory_old, uncondIndHistory_old, global_path_history_MHRP);
}

uint64_t chirp::make_signature(uint64_t pc, const std::string& name, uint64_t cond_hist, uint64_t uncond_hist, uint64_t path_hist)
{
    uint64_t mixed = 0;
    if (name.compare("cpu0_ITLB") == 0) {
        mixed = (pc) ^ (cond_hist) ^ (uncond_hist) ^ (path_hist);
    } else if (name.compare("cpu0_DTLB") == 0) {
        mixed = (pc) ^ (cond_hist) ^ (uncond_hist) ^ (path_hist);
    } else if (name.compare("cpu0_STLB") == 0) {
        mixed = (pc) ^ (cond_hist) ^ (uncond_hist) ^ (path_hist);
    }
    return (mixed) & ((1 << signature_bits) - 1);
}

// Kept for backward compatibility with non-EXPAND_PACKET builds
void chirp::set_branch_history(uint64_t cond, uint64_t uncond, uint64_t global)
{
    condHistory_old = cond;
    uncondIndHistory_old = uncond;
    global_path_history_MHRP = global;
}

long chirp::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                        champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                        ,
                        CACHE::repl_pol_xargs xargs
#endif
)
{
    (void)instr_id;
    (void)full_addr;
#if defined(EXPAND_PACKET)
    set_branch_history(xargs.cond_history, xargs.uncond_ind_history, xargs.global_path_history);
#endif

    nvict++;
    long way = intern_->NUM_WAY;
    unsigned int trace = make_signature(ip.to<uint64_t>(), intern_->NAME, condHistory_old, uncondIndHistory_old, global_path_history_MHRP);

    // not sure when and why we bypass
    bool prediction_bypass;
    int pred_confidence = _predTable.get_prediction(static_cast<int>(module_type), trace);
    if (pred_confidence > threshold_bypass) {
        prediction_bypass = true;
    } else {
        prediction_bypass = false;
    }

    if (prediction_bypass == true) {
        bypass_cnt++;
    } else {
        // check for invalid entries
        for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
            if (!current_set[i].valid) {
                empty_cache_cnt++;
                way = i;
                break;
            }
        }
        // if no invalid entry was found, look for predicted dead blocks
        if (way == intern_->NUM_WAY) {
            for (uint32_t i = 0; i < intern_->NUM_WAY; i++) {
                if (is_dead[set * intern_->NUM_WAY + i]) {
                    cache_dead_victim_cnt++;
                    way = i;
                    break;
                }
            }
        }
        // if not found, lookup LRU victim
        if (way == intern_->NUM_WAY) {
            auto begin = std::next(std::begin(last_used_cycles), set * intern_->NUM_WAY);
            auto end = std::next(begin, intern_->NUM_WAY);

            // Find the way whose last use cycle is most distant
            auto victim = std::min_element(begin, end);
            assert(begin <= victim);
            assert(victim < end);
            way = std::distance(begin, victim);
        }
    }
    return way;
}

void chirp::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                    champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                    ,
                                    CACHE::repl_pol_xargs xargs
#endif
)
{
#if defined(EXPAND_PACKET)
    set_branch_history(xargs.cond_history, xargs.uncond_ind_history, xargs.global_path_history);
#endif

    unsigned int trace = make_signature(ip.to<uint64_t>(), intern_->NAME, condHistory_old, uncondIndHistory_old, global_path_history_MHRP);
    int pred_confidence = _predTable.get_prediction(static_cast<int>(module_type), trace);
    if (pred_confidence >= cache_thresh) {
        is_dead[set * intern_->NUM_WAY + way] = true;
    } else {
        is_dead[set * intern_->NUM_WAY + way] = false;
    }

    if (pred_confidence >= cache_thresh) {
        if (set % sampler_modulus == 0) {
            cache_samp_dead_cnt++;
        } else {
            cache_non_samp_dead_cnt++;
        }
    }

    // Update Sampler
    int sampler_set_idx = set / sampler_modulus;
    if (sampler_set_idx < static_cast<int>(sampler_sets.size())) {
        auto& blocks = sampler_sets[sampler_set_idx].blocks;
        uint64_t partial_tag = full_addr.to<uint64_t>();
        bool matchFound = false;
        bool emptyFound = false;
        bool deadFound = false;
        bool feedback = false;
        uint32_t victim_idx = _sampler_assoc; // dummy val
        uint64_t trace_current = make_signature(ip.to<uint64_t>(), intern_->NAME);

        for (uint32_t i = 0; i < _sampler_assoc; i++) {
            if ((blocks[i].valid == true) && (blocks[i].tag == partial_tag)) {
                matchFound = true;
                victim_idx = i;
                matched_samp_cnt++;
                feedback = true;
                _predTable.block_is_dead(static_cast<int>(module_type), blocks[i].trace, false);
                break;
            }
        }

        if (matchFound == false) {
            for (uint32_t i = 0; i < _sampler_assoc; i++) {
                if (blocks[i].valid == false) {
                    emptyFound = true;
                    victim_idx = i;
                    empty_sampler_cnt++;
                    break;
                }
            }
            if (emptyFound == false) {
                uint32_t deadest = _sampler_assoc;
                for (uint32_t i = 0; i < _sampler_assoc; i++) {
                    if (blocks[i].prediction == true) {
                        deadFound = true;
                        victim_idx = i;
                        deadest = i;
                        break;
                    }
                }
                if (deadFound == true) {
                    samp_dead_victim_cnt++;
                    feedback = true;
                }
            }

            if (deadFound == false && emptyFound == false) {
                uint32_t j;
                for (j = 0; j < _sampler_assoc; j++) {
                    if (blocks[j].lru_stack_position == (_sampler_assoc - 1)) {
                        nevict_samp++;
                        feedback = true;
                        lru_sampler_cnt++;
                        break;
                    }
                }
                assert(j < _sampler_assoc);
                victim_idx = j;
            }
            blocks[victim_idx].tag = partial_tag;
            blocks[victim_idx].valid = true;
        }
        blocks[victim_idx].trace = trace_current;
        pred_confidence = _predTable.get_prediction(static_cast<int>(module_type), trace_current);
        if (pred_confidence >= cache_thresh) {
            blocks[victim_idx].prediction = true;
        } else {
            blocks[victim_idx].prediction = false;
        }
        blocks[victim_idx].conf_val = pred_confidence;

        unsigned int position = blocks[victim_idx].lru_stack_position;
        for (uint32_t i = 0; i < _sampler_assoc; i++)
            if (blocks[i].lru_stack_position < position)
                blocks[i].lru_stack_position++;
        blocks[victim_idx].lru_stack_position = 0;
    }

    // Update prediction Table
    if (set == last_set) {
        table_update_flag = false;
    } else {
        table_update_flag = true;
    }
    last_set = set;

    // Update LRU
    if (!hit || type != access_type::WRITE) // Skip this for writeback hits
        last_used_cycles[set * intern_->NUM_WAY + way] = intern_->current_cycle();

}

void chirp::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                  champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                  ,
                                  CACHE::repl_pol_xargs xargs
#endif
)
{
    // CHiRP has no special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void chirp::replacement_final_stats()
{
    std::cout << intern_->NAME << " CHiRP Stats:" << std::endl;
    std::cout << "\tbypass_cnt: " << bypass_cnt << std::endl;
    std::cout << "\tcache_dead_victim_cnt: " << cache_dead_victim_cnt << std::endl;
    std::cout << "\tmatched_samp_cnt: " << matched_samp_cnt << std::endl;
    std::cout << "\tempty_sampler_cnt: " << empty_sampler_cnt << std::endl;
    std::cout << "\tlru_sampler_cnt: " << lru_sampler_cnt << std::endl;
    std::cout << "\tempty_cache_cnt: " << empty_cache_cnt << std::endl;
    std::cout << "\tcache_samp_dead_cnt: " << cache_samp_dead_cnt << std::endl;
}