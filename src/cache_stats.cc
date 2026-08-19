#include "cache_stats.h"

cache_stats operator-(cache_stats lhs, cache_stats rhs)
{
  cache_stats result;
  result.pf_requested = lhs.pf_requested - rhs.pf_requested;
  result.pf_issued = lhs.pf_issued - rhs.pf_issued;
  result.pf_useful = lhs.pf_useful - rhs.pf_useful;
  result.pf_useless = lhs.pf_useless - rhs.pf_useless;
  result.pf_fill = lhs.pf_fill - rhs.pf_fill;

#if defined(ENABLE_PAGE_CROSSING_STATS)
  result.pf_crossing_pages_tlb_hit = lhs.pf_crossing_pages_tlb_hit - rhs.pf_crossing_pages_tlb_hit;
  result.pf_crossing_pages_tlb_miss = lhs.pf_crossing_pages_tlb_miss - rhs.pf_crossing_pages_tlb_miss;
#endif

  result.hits = lhs.hits - rhs.hits;
  result.misses = lhs.misses - rhs.misses;

  result.total_miss_latency_cycles = lhs.total_miss_latency_cycles - rhs.total_miss_latency_cycles;

#if defined(EXPAND_PACKET)
  result.ihits = lhs.ihits - rhs.ihits;
  result.imisses = lhs.imisses - rhs.imisses;
  result.dhits = lhs.dhits - rhs.dhits;
  result.dmisses = lhs.dmisses - rhs.dmisses;
  result.ithits = lhs.ithits - rhs.ithits;
  result.itmisses = lhs.itmisses - rhs.itmisses;
  result.dthits = lhs.dthits - rhs.dthits;
  result.dtmisses = lhs.dtmisses - rhs.dtmisses;

  result.total_imiss_latency = lhs.total_imiss_latency - rhs.total_imiss_latency;
  result.total_dmiss_latency = lhs.total_dmiss_latency - rhs.total_dmiss_latency;
  result.total_itmiss_latency = lhs.total_itmiss_latency - rhs.total_itmiss_latency;
  result.total_dtmiss_latency = lhs.total_dtmiss_latency - rhs.total_dtmiss_latency;
#endif

  return result;
}
