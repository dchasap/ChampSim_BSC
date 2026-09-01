#ifndef PREFETCHER_NEXT_LINE_INSTR_H
#define PREFETCHER_NEXT_LINE_INSTR_H

#include <cassert>
#include <cstdint>
#include <vector>

#include "address.h"
#include "champsim.h"
#include "modules.h"
#include "util/to_underlying.h"

/*
 * Next-line prefetcher for the instruction stream.
 *
 * The old next_line_instr prefetcher assumes addr == ip (an invariant
 * for instruction prefetchers in the old code path) and prefetches
 * addr + BLOCK_SIZE (i.e., addr + (1 << LOG2_BLOCK_SIZE)).
 *
 * The block size is taken from the global LOG2_BLOCK_SIZE constant.
 */

struct next_line_instr : public champsim::modules::prefetcher {
  using prefetcher::prefetcher;
  uint64_t block_size = (1ULL << LOG2_BLOCK_SIZE);

  champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type, champsim::modules::pf_meta_t metadata_in);

  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);

  // Header-only candidate interface for use by pf_combiner
  inline std::vector<champsim::address> get_prefetch_candidates(champsim::address addr, champsim::address ip, uint8_t cache_hit,
                                                                 bool useful_prefetch, access_type type,
champsim::modules::pf_meta_t metadata_in);

  inline void record_prefetch_issued(champsim::address /*pf_addr*/) {}
  inline void notify_enqueue_failed() {}
  inline double get_mshr_gate_pct() const { return 100.0; }
};

inline champsim::modules::pf_meta_t next_line_instr::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
champsim::modules::pf_meta_t metadata_in)
{
  auto candidates = get_prefetch_candidates(addr, ip, cache_hit, useful_prefetch, type, metadata_in);
  for (auto pf_addr : candidates) {
    prefetch_line(pf_addr, true, metadata_in);
  }
  return metadata_in;
}

inline champsim::modules::pf_meta_t next_line_instr::prefetcher_cache_fill(champsim::address /*addr*/, long /*set*/, long /*way*/, uint8_t /*prefetch*/,
                                                champsim::address /*evicted_addr*/,
champsim::modules::pf_meta_t metadata_in)
{
  return metadata_in;
}

inline std::vector<champsim::address> next_line_instr::get_prefetch_candidates(champsim::address addr, champsim::address ip, uint8_t cache_hit,
                                                                             bool useful_prefetch, access_type type,
champsim::modules::pf_meta_t metadata_in)
{
  (void)ip; (void)cache_hit; (void)useful_prefetch; (void)type; (void)metadata_in;

  // Invariant for instruction prefetchers
  assert(addr == ip);
  return {champsim::address{addr + block_size}};
}

#endif
