#include "core_stats.h"

cpu_stats operator-(cpu_stats lhs, cpu_stats rhs)
{
  lhs.begin_instrs -= rhs.begin_instrs;
  lhs.begin_cycles -= rhs.begin_cycles;
  lhs.end_instrs -= rhs.end_instrs;
  lhs.end_cycles -= rhs.end_cycles;
  lhs.total_rob_occupancy_at_branch_mispredict -= rhs.total_rob_occupancy_at_branch_mispredict;

  lhs.total_branch_types -= rhs.total_branch_types;
  lhs.branch_type_misses -= rhs.branch_type_misses;

#if defined(ENABLE_MULTIPLE_PAGE_SIZE)
  lhs.instr_large_page_accesses -= rhs.instr_large_page_accesses;
  lhs.instr_small_page_accesses -= rhs.instr_small_page_accesses;
  lhs.data_large_page_accesses -= rhs.data_large_page_accesses;
  lhs.data_small_page_accesses -= rhs.data_small_page_accesses;
#endif

  return lhs;
}
