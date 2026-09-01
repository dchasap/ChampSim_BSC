#ifndef CHILD_IDEAL_PF_H
#define CHILD_IDEAL_PF_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "cache.h"
#include "champsim.h"
#include "modules.h"
#include "env_var.h"
#include "util/to_underlying.h"

struct child_ideal : public champsim::modules::prefetcher {
  using DiscoveryKey = std::pair<uint64_t, std::size_t>; // (vpn, level)
  using PredictionKey = uint64_t; // parent_full_addr only

  std::map<DiscoveryKey, uint64_t> table_a;
  std::map<PredictionKey, uint64_t> table_b;

  bool train_on_hit;
  bool issue_on_hit;
  bool fill_this_level;  // Control where prefetches install: true=this cache, false=lower level only
  
  uint32_t mshr_gate_pct;
  uint64_t offset_bits;

  // Statistics
  uint64_t access_count = 0;
  uint64_t access_tick = 0;
  uint64_t prediction_lookups = 0;
  uint64_t prediction_hits = 0;
  uint64_t prediction_misses = 0;
  uint64_t training_updates = 0;
  uint64_t discovery_updates = 0;
  uint64_t prefetches_issued = 0;
  uint64_t mshr_gate_blocked = 0;
  uint64_t total_ptes_observed = 0;
  std::unordered_set<uint64_t> unique_ptes;
  std::unordered_set<uint64_t> unique_parents;
  std::unordered_set<uint64_t> unique_children;

  struct PairHash {
    size_t operator()(const std::pair<uint64_t, uint64_t>& p) const noexcept {
      return std::hash<uint64_t>{}(p.first) ^ (std::hash<uint64_t>{}(p.second) << 1);
    }
  };
  struct PairEq {
    bool operator()(const std::pair<uint64_t, uint64_t>& a, const std::pair<uint64_t, uint64_t>& b) const noexcept {
      return a.first == b.first && a.second == b.second;
    }
  };
  std::unordered_set<std::pair<uint64_t, uint64_t>, PairHash, PairEq> unique_parent_child_pairs;
  std::unordered_set<std::pair<uint64_t, uint64_t>, PairHash, PairEq> unique_parent_child_pairs_with_replacement;

  // Constructor
  explicit child_ideal(CACHE* cache);

  bool should_train_on_hit() const { return train_on_hit; }
  bool should_issue_on_hit() const { return issue_on_hit; }
  bool get_fill_this_level() const { return fill_this_level; }
  uint32_t get_mshr_gate_pct() const { return mshr_gate_pct; }

  // Interface for pf_combiner
  inline std::vector<uint64_t> get_prefetch_candidates(uint64_t parent_addr, std::size_t translation_level, uint64_t translated_vpn);
  inline void record_prefetch_issued(uint64_t pf_addr);
  inline void notify_enqueue_failed() { mshr_gate_blocked++; }
  inline void notify_mshr_gate_blocked() { mshr_gate_blocked++; }
  inline void observe_access(uint64_t address, std::size_t translation_level, uint64_t translated_vpn);
  void print_stats();

  champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                    champsim::modules::pf_meta_t metadata_in);
  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);
  void prefetcher_cycle_operate() {}
  void prefetcher_final_stats() { print_stats(); }
};

// Inline implementations for pf_combiner compatibility
#if defined(EXPAND_PREFETCH)
inline std::vector<uint64_t> child_ideal::get_prefetch_candidates(uint64_t parent_addr, std::size_t translation_level, uint64_t translated_vpn)
{
  (void)translation_level;
  (void)translated_vpn;
#else
inline std::vector<uint64_t> child_ideal::get_prefetch_candidates(uint64_t parent_addr, std::size_t translation_level, uint64_t /*translated_vpn*/)
{
  (void)translation_level;
#endif
  prediction_lookups++;

  auto it = table_b.find(parent_addr);
  if (it == table_b.end()) {
    prediction_misses++;
    return {};
  }

  prediction_hits++;

  // table_b stores child PTE addresses
  uint64_t child_pte_addr = it->second;
  if (child_pte_addr == 0) {
    prediction_misses++;
    return {};
  }

  // Return single candidate (the predicted child PTE)
  return {child_pte_addr};
}

#if defined(EXPAND_PREFETCH)
inline void child_ideal::observe_access(uint64_t address, std::size_t translation_level, uint64_t translated_vpn)
{
  (void)translated_vpn;
#else
inline void child_ideal::observe_access(uint64_t address, std::size_t translation_level, uint64_t /*translated_vpn*/)
{
#endif
  access_count++;
  access_tick++;

  // In child_ideal: we discover parent->child relationships
  // address is the parent PTE address we just accessed
  // translation_level is which level of page table we're at

  // Extract VPN from the address (for determining page alignment)
  uint64_t addr_vpn = address >> LOG2_PAGE_SIZE;

  // Store the parent->child relationship
  // For simplicity, we use table_a to learn which PTEs are children of which parent
  DiscoveryKey dk = {addr_vpn, translation_level};

  // The child PTE should be at a different VPN (since we're prefetching a child PTE)
  // For now, we just store the parent info
  if (table_a.find(dk) == table_a.end()) {
    discovery_updates++;
    table_a[dk] = 0; // Initialize
  }
}

inline void child_ideal::record_prefetch_issued(uint64_t pf_addr)
{
  prefetches_issued++;
  unique_children.insert(pf_addr);
}

// Implementation
inline champsim::modules::pf_meta_t child_ideal::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                               champsim::modules::pf_meta_t metadata_in)
{
  (void)ip; (void)useful_prefetch;
  
  // Only activate on TRANSLATION-type accesses (PTW probes)
  if (type != access_type::TRANSLATION)
    return metadata_in;

#if defined(EXPAND_PREFETCH)
  // Extract translation_level from lower 4 bits and VPN from upper bits of metadata
  const std::size_t translation_level = static_cast<std::size_t>(metadata_in & 0xF);
  const uint64_t vpn_mask_60 = (1ULL << 60) - 1;
  const uint64_t translated_vpn = (metadata_in >> 4) & vpn_mask_60;
#else
  const std::size_t translation_level = 0;
  const uint64_t translated_vpn = 0;
#endif

  if (cache_hit) {
    if (!should_train_on_hit()) {
      return metadata_in;
    }

    if (should_issue_on_hit()) {
      if (intern_->get_mshr_occupancy_ratio() * 100 >= get_mshr_gate_pct()) {
        notify_mshr_gate_blocked();
        observe_access(addr.to<uint64_t>(), translation_level, translated_vpn);
        return metadata_in;
      }

      auto candidates = get_prefetch_candidates(addr.to<uint64_t>(), translation_level, translated_vpn);
      for (auto pf_addr : candidates) {
        if (pf_addr == 0) continue;
        if (prefetch_line(champsim::address{pf_addr}, get_fill_this_level(), 0)) {
          record_prefetch_issued(pf_addr);
        } else {
          notify_enqueue_failed();
        }
      }
    } else {
      observe_access(addr.to<uint64_t>(), translation_level, translated_vpn);
    }
  } else {
    if (intern_->get_mshr_occupancy_ratio() * 100 >= get_mshr_gate_pct()) {
      notify_mshr_gate_blocked();
      return metadata_in;
    }

    auto candidates = get_prefetch_candidates(addr.to<uint64_t>(), translation_level, translated_vpn);
    for (auto pf_addr : candidates) {
      if (pf_addr == 0) continue;
      unique_ptes.insert(addr.to<uint64_t>());
      if (prefetch_line(champsim::address{pf_addr}, get_fill_this_level(), 0)) {
        record_prefetch_issued(pf_addr);
        unique_parent_child_pairs.insert({addr.to<uint64_t>(), pf_addr});
      } else {
        notify_enqueue_failed();
      }
    }
  }

  training_updates++;
  return metadata_in;
}

inline champsim::modules::pf_meta_t child_ideal::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in)
{
  (void)addr; (void)set; (void)way; (void)prefetch; (void)evicted_addr;
  return metadata_in;
}

#endif // CHILD_IDEAL_PF_H
