#ifndef PF_COMBINER_H
#define PF_COMBINER_H

#include <iostream>

#include "cache.h"
#include "champsim.h"
#include "modules.h"
#include "env_var.h"

#include "../child_ideal/child_ideal_pf.h"
#include "../next_line/next_line.h"

// PFCombiner: Combines child_ideal (PTE prefetcher) with another regular prefetcher
// - child_ideal handles TRANSLATION accesses (page table walks) 
// - Regular prefetcher (e.g., next_line) handles normal accesses
// This is a header-only prefetcher so it can include child_ideal's internals

struct pf_combiner : public champsim::modules::prefetcher {
  child_ideal child_pf;
  next_line regular_pf;
  
public:
  explicit pf_combiner(CACHE* cache) : champsim::modules::prefetcher(cache), 
                                       child_pf(cache), 
                                       regular_pf(cache)
  {
    std::cout << "PFCombiner: initialized with child_ideal + next_line" << std::endl;
  }

  champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                    champsim::modules::pf_meta_t metadata_in);
  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);

  void prefetcher_cycle_operate() {}
  void prefetcher_final_stats();
};

// Implementation
inline champsim::modules::pf_meta_t pf_combiner::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                              champsim::modules::pf_meta_t metadata_in)
{
  // Handle TRANSLATION accesses with child_ideal (PTE prefetcher)
  if (type == access_type::TRANSLATION) {
    // Extract translation_level and VPN from metadata (encoded by ptw.cc under EXPAND_PREFETCH)
#if defined(EXPAND_PREFETCH)
    const std::size_t translation_level = static_cast<std::size_t>(metadata_in & 0xF);
    const uint64_t vpn_mask_60 = (1ULL << 60) - 1;
    const uint64_t translated_vpn = (metadata_in >> 4) & vpn_mask_60;
#else
    const std::size_t translation_level = 0;
    const uint64_t translated_vpn = 0;
#endif

    auto candidates = child_pf.get_prefetch_candidates(addr.to<uint64_t>(), translation_level, translated_vpn);

    if (!candidates.empty()) {
      // MSHR gate check for child prefetcher
      if (intern_->get_mshr_occupancy_ratio() * 100 >= child_pf.get_mshr_gate_pct()) {
        return metadata_in;  // Back off if MSHR too full
      }

      for (auto pf_addr : candidates) {
        if (pf_addr == 0) continue;
        // Use child_pf's configurable fill_this_level
        if (prefetch_line(champsim::address{pf_addr}, child_pf.get_fill_this_level(), 0)) {
          child_pf.record_prefetch_issued(pf_addr);
        } else {
          child_pf.notify_enqueue_failed();
        }
      }
    }
    
    return metadata_in;
  } 
  
  // Handle regular accesses with next_line prefetcher
  return regular_pf.prefetcher_cache_operate(addr, ip, cache_hit, useful_prefetch, type, metadata_in);
}

inline champsim::modules::pf_meta_t pf_combiner::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in)
{
  (void)addr; (void)set; (void)way; (void)prefetch; (void)evicted_addr;
  return metadata_in;
}

inline void pf_combiner::prefetcher_final_stats()
{
  child_pf.print_stats();
  std::cout << "PFCombiner Regular Prefetcher Stats: (handled by next_line)" << std::endl;
}

#endif // PF_COMBINER_H
