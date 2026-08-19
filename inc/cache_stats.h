#ifndef CACHE_STATS_H
#define CACHE_STATS_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <utility>

#include "channel.h"
#include "event_counter.h"

struct cache_stats {
  std::string name;
  // prefetch stats
  uint64_t pf_requested = 0;
  uint64_t pf_issued = 0;
  uint64_t pf_useful = 0;
  uint64_t pf_useless = 0;
  uint64_t pf_fill = 0;

  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> hits = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> misses = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> miss_merge = {};
  champsim::stats::event_counter<std::pair<access_type, std::remove_cv_t<decltype(NUM_CPUS)>>> fill = {};

  long total_miss_latency_cycles{};

#if defined(ENABLE_PAGE_CROSSING_STATS)
  uint64_t pf_crossing_pages_tlb_hit = 0;
  uint64_t pf_crossing_pages_tlb_miss = 0;
#endif

#if defined(EXPAND_PACKET)
  // Categorized hit/miss statistics
  // i = instruction (non-TLB), d = data (non-TLB)
  // it = instruction TLB, dt = data TLB
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> ihits = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> imisses = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dhits = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dmisses = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> ithits = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> itmisses = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dthits = {};
  champsim::stats::event_counter<std::remove_cv_t<decltype(NUM_CPUS)>> dtmisses = {};

  // Categorized miss latencies
  long total_imiss_latency{};
  long total_dmiss_latency{};
  long total_itmiss_latency{};
  long total_dtmiss_latency{};
#endif
};

cache_stats operator-(cache_stats lhs, cache_stats rhs);

#endif
