#include "mockingjay.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <unordered_map>
#include <vector>

#include "cache.h"
#include "champsim.h"
#include "msl/bits.h"

namespace
{
// Helper functions from the old implementation
uint64_t CRC_HASH(uint64_t _blockAddress)
{
    static const uint64_t crcPolynomial = 3988292384ULL;
    uint64_t _returnVal = _blockAddress;
    for (unsigned int i = 0; i < 3; i++)
        _returnVal = ((_returnVal & 1) == 1) ? ((_returnVal >> 1) ^ crcPolynomial) : (_returnVal >> 1);
    return _returnVal;
}

uint64_t get_pc_signature(uint64_t pc, bool hit, bool prefetch, uint32_t core, int pc_signature_bits)
{
    if (NUM_CPUS == 1) {
        pc = pc << 1;
        if (hit) {
            pc = pc | 1;
        }
        pc = pc << 1;
        if (prefetch) {
            pc = pc | 1;
        }
        pc = CRC_HASH(pc);
        pc = (pc << (64 - pc_signature_bits)) >> (64 - pc_signature_bits);
    } else {
        pc = pc << 1;
        if (prefetch) {
            pc = pc | 1;
        }
        pc = pc << 2;
        pc = pc | core;
        pc = CRC_HASH(pc);
        pc = (pc << (64 - pc_signature_bits)) >> (64 - pc_signature_bits);
    }
    return pc;
}

uint32_t get_sampled_cache_index(uint64_t full_addr, int log2_block_size, int log2_num_set, int log2_sampled_cache_sets)
{
    full_addr = full_addr >> log2_block_size;
    full_addr = (full_addr << (64 - (log2_sampled_cache_sets + log2_num_set))) >> (64 - (log2_sampled_cache_sets + log2_num_set));
    return static_cast<uint32_t>(full_addr);
}

uint64_t get_sampled_cache_tag(uint64_t x, int log2_block_size, int log2_num_set, int log2_sampled_cache_sets, int sampled_cache_tag_bits)
{
    x >>= log2_num_set + log2_block_size + log2_sampled_cache_sets;
    x = (x << (64 - sampled_cache_tag_bits)) >> (64 - sampled_cache_tag_bits);
    return x;
}

int time_elapsed(int global, uint64_t local, int timestamp_bits)
{
    if (global >= static_cast<int>(local)) {
        return global - static_cast<int>(local);
    }
    global = global + (1 << timestamp_bits);
    return global - static_cast<int>(local);
}

int temporal_difference(int init, int sample, double temp_difference, int inf_rd)
{
    if (sample > init) {
        double diff = static_cast<double>(sample - init);
        diff = diff * temp_difference;
        diff = std::min(1.0, diff);
        return std::min(init + static_cast<int>(diff), inf_rd);
    } else if (sample < init) {
        double diff = static_cast<double>(init - sample);
        diff = diff * temp_difference;
        diff = std::min(1.0, diff);
        return std::max(init - static_cast<int>(diff), 0);
    } else {
        return init;
    }
}

int increment_timestamp(int input, int timestamp_bits)
{
    input++;
    input = input % (1 << timestamp_bits);
    return input;
}

void detrain(std::unordered_map<uint32_t, int>& rdp, int signature, int inf_rd)
{
    if (rdp.count(signature)) {
        rdp[signature] = std::min(rdp[signature] + 1, inf_rd);
    } else {
        rdp[signature] = inf_rd;
    }
}

bool is_sampled_set(long set, int log2_num_set, int log2_sampled_sets)
{
    int mask_length = log2_num_set - log2_sampled_sets;
    int mask = (1 << mask_length) - 1;
    return (static_cast<int>(set) & mask) == ((static_cast<int>(set) >> (log2_num_set - mask_length)) & mask);
}

int search_sampled_cache(const std::vector<mockingjay::SampledCacheLine*>& sampled_set, uint64_t blockAddress)
{
    for (size_t way = 0; way < sampled_set.size(); way++) {
        if (sampled_set[way]->valid && (sampled_set[way]->tag == blockAddress)) {
            return static_cast<int>(way);
        }
    }
    return -1;
}
} // namespace

mockingjay::mockingjay(CACHE* cache) : mockingjay(cache, cache->NUM_SET, cache->NUM_WAY) {}

mockingjay::mockingjay(CACHE* cache, long sets_, long ways_) : replacement(cache)
{
    (void)sets_;
    (void)ways_;

    LOG2_NUM_SET = static_cast<int>(std::log2(intern_->NUM_SET));
    LOG2_LLC_SIZE = LOG2_NUM_SET + static_cast<int>(std::log2(intern_->NUM_WAY)) + LOG2_BLOCK_SIZE;
    LOG2_SAMPLED_SETS = LOG2_LLC_SIZE - 16;

    HISTORY = 8;
    GRANULARITY = 8;

    INF_RD = intern_->NUM_WAY * HISTORY - 1;
    INF_ETR = (intern_->NUM_WAY * HISTORY / GRANULARITY) - 1;
    MAX_RD = INF_RD - 22;

    SAMPLED_CACHE_WAYS = 5;
    LOG2_SAMPLED_CACHE_SETS = 4;
    SAMPLED_CACHE_TAG_BITS = 31 - LOG2_LLC_SIZE;
    PC_SIGNATURE_BITS = LOG2_LLC_SIZE - 10;
    TIMESTAMP_BITS = 8;

    TEMP_DIFFERENCE = 1.0 / 16.0;
    FLEXMIN_PENALTY = 2.0 - std::log2(NUM_CPUS) / 4.0;

    etr = std::vector<std::vector<int>>(intern_->NUM_SET);
    for (uint32_t i = 0; i < intern_->NUM_SET; i++) {
        etr[i] = std::vector<int>(intern_->NUM_WAY);
    }

    etr_clock = std::vector<int>(intern_->NUM_SET);
    current_timestamp = std::vector<int>(intern_->NUM_SET);

    for (uint32_t i = 0; i < intern_->NUM_SET; i++) {
        etr_clock[i] = GRANULARITY;
        current_timestamp[i] = 0;
    }

    for (uint32_t set = 0; set < intern_->NUM_SET; set++) {
        if (is_sampled_set(set, LOG2_NUM_SET, LOG2_SAMPLED_SETS)) {
            int modifier = 1 << LOG2_NUM_SET;
            int limit = 1 << LOG2_SAMPLED_CACHE_SETS;
            for (int i = 0; i < limit; i++) {
                sampled_cache[set + modifier * i] = std::vector<SampledCacheLine*>(SAMPLED_CACHE_WAYS);
                for (int j = 0; j < SAMPLED_CACHE_WAYS; j++) {
                    sampled_cache[set + modifier * i][j] = new SampledCacheLine();
                    sampled_cache[set + modifier * i][j]->valid = false;
                }
            }
        }
    }
}

void mockingjay::initialize_replacement()
{
    // Initialization is done in constructor
}

long mockingjay::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set, const champsim::cache_block* current_set, champsim::address ip,
                             champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                             ,
                             CACHE::repl_pol_xargs xargs
#endif
)
{
    (void)triggering_cpu;
    (void)instr_id;
    (void)current_set;
    (void)ip;
    (void)full_addr;
    (void)type;
#if defined(EXPAND_PACKET)
    (void)xargs;
#endif

    // Don't modify this code or put anything above it;
    // if there's an invalid block, we don't need to evict any valid ones
    for (uint32_t way = 0; way < intern_->NUM_WAY; way++) {
        if (!current_set[way].valid) {
            return way;
        }
    }

    // Your eviction policy goes here
    int max_etr = 0;
    int victim_way = 0;
    for (uint32_t way = 0; way < intern_->NUM_WAY; way++) {
        if (abs(etr[set][way]) > max_etr ||
            (abs(etr[set][way]) == max_etr &&
                etr[set][way] < 0)) {
            max_etr = abs(etr[set][way]);
            victim_way = way;
        }
    }

    uint32_t pc_signature =
        static_cast<uint32_t>(get_pc_signature(ip.to<uint64_t>(), false, type == access_type::PREFETCH, triggering_cpu, PC_SIGNATURE_BITS));
    if (type != access_type::WRITE && rdp.count(pc_signature) &&
        (rdp.at(pc_signature) > MAX_RD || rdp.at(pc_signature) / GRANULARITY > max_etr)) {
        return intern_->NUM_WAY;
    }

    return victim_way;
}

void mockingjay::update_replacement_state(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                          champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                          ,
                                          CACHE::repl_pol_xargs xargs
#endif
)
{
#if defined(EXPAND_PACKET)
    (void)xargs;
#endif

    if (type == access_type::WRITE) {
        if (!hit) {
            etr[set][way] = -INF_ETR;
        }
        return;
    }

    uint32_t pc = static_cast<uint32_t>(get_pc_signature(ip.to<uint64_t>(), hit, type == access_type::PREFETCH, triggering_cpu, PC_SIGNATURE_BITS));

    if (is_sampled_set(set, LOG2_NUM_SET, LOG2_SAMPLED_SETS)) {
        uint32_t sampled_cache_index = get_sampled_cache_index(full_addr.to<uint64_t>(), LOG2_BLOCK_SIZE, LOG2_NUM_SET, LOG2_SAMPLED_CACHE_SETS);
        uint64_t sampled_cache_tag = get_sampled_cache_tag(full_addr.to<uint64_t>(), LOG2_BLOCK_SIZE, LOG2_NUM_SET, LOG2_SAMPLED_CACHE_SETS, SAMPLED_CACHE_TAG_BITS);
        int sampled_cache_way = search_sampled_cache(sampled_cache[sampled_cache_index], sampled_cache_tag);

        if (sampled_cache_way > -1) {
            uint32_t last_signature = static_cast<uint32_t>(sampled_cache[sampled_cache_index][sampled_cache_way]->signature);
            uint64_t last_timestamp = sampled_cache[sampled_cache_index][sampled_cache_way]->timestamp;
            int sample = time_elapsed(current_timestamp[set], last_timestamp, TIMESTAMP_BITS);

            if (sample <= INF_RD) {
                if (type == access_type::PREFETCH) {
                    sample = static_cast<int>(static_cast<double>(sample) * FLEXMIN_PENALTY);
                }
                if (rdp.count(last_signature)) {
                    int init = rdp.at(last_signature);
                    rdp[last_signature] = temporal_difference(init, sample, TEMP_DIFFERENCE, INF_RD);
                } else {
                    rdp[last_signature] = sample;
                }

                sampled_cache[sampled_cache_index][sampled_cache_way]->valid = false;
            }
        }

        int lru_way = -1;
        int lru_rd = -1;
        for (int w = 0; w < SAMPLED_CACHE_WAYS; w++) {
            if (sampled_cache[sampled_cache_index][w]->valid == false) {
                lru_way = w;
                lru_rd = INF_RD + 1;
                continue;
            }

            uint64_t last_timestamp = sampled_cache[sampled_cache_index][w]->timestamp;
            int sample = time_elapsed(current_timestamp[set], last_timestamp, TIMESTAMP_BITS);
            if (sample > INF_RD) {
                lru_way = w;
                lru_rd = INF_RD + 1;
                detrain(rdp, static_cast<uint32_t>(sampled_cache[sampled_cache_index][w]->signature), INF_RD);
            } else if (sample > lru_rd) {
                lru_way = w;
                lru_rd = sample;
            }
        }
        if (lru_way >= 0)
            detrain(rdp, static_cast<uint32_t>(sampled_cache[sampled_cache_index][lru_way]->signature), INF_RD);

        for (int w = 0; w < SAMPLED_CACHE_WAYS; w++) {
            if (sampled_cache[sampled_cache_index][w]->valid == false) {
                sampled_cache[sampled_cache_index][w]->valid = true;
                sampled_cache[sampled_cache_index][w]->signature = pc;
                sampled_cache[sampled_cache_index][w]->tag = sampled_cache_tag;
                sampled_cache[sampled_cache_index][w]->timestamp = current_timestamp[set];
                break;
            }
        }

        current_timestamp[set] = increment_timestamp(current_timestamp[set], TIMESTAMP_BITS);
    }

    if (etr_clock[set] == GRANULARITY) {
        for (uint32_t w = 0; w < intern_->NUM_WAY; w++) {
            if ((uint32_t)w != way && abs(etr[set][w]) < INF_ETR) {
                etr[set][w]--;
            }
        }
        etr_clock[set] = 0;
    }
    etr_clock[set]++;

    if (way < intern_->NUM_WAY) {
        if (!rdp.count(pc)) {
            if (NUM_CPUS == 1) {
                etr[set][way] = 0;
            } else {
                etr[set][way] = INF_ETR;
            }
        } else {
            if (rdp.at(pc) > MAX_RD) {
                etr[set][way] = INF_ETR;
            } else {
                etr[set][way] = rdp.at(pc) / GRANULARITY;
            }
        }
    }
}

void mockingjay::replacement_cache_fill(uint32_t triggering_cpu, long set, long way, champsim::address full_addr, champsim::address ip,
                                        champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                        ,
                                        CACHE::repl_pol_xargs xargs
#endif
)
{
    // Mockingjay has no special cache fill handling beyond update_replacement_state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, 0);
#endif
}

void mockingjay::replacement_final_stats()
{
    // Print RDP stats if needed
    // The original code printed sorted frequency data
}