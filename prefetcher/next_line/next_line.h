#ifndef PREFETCHER_NEXT_LINE_H
#define PREFETCHER_NEXT_LINE_H

#include <cstdint>
#include <vector>

#include "address.h"
#include "modules.h"

// Next line prefetcher - header-only implementation
// Returns the next cache line address as a prefetch candidate

struct next_line : public champsim::modules::prefetcher {
  using prefetcher::prefetcher;
  champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                    champsim::modules::pf_meta_t metadata_in);
  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);

  // void prefetcher_initialize();
  // void prefetcher_branch_operate(champsim::address ip, uint8_t branch_type, champsim::address branch_target) {}
  // void prefetcher_cycle_operate() {}
  // void prefetcher_final_stats() {}
};

#endif
