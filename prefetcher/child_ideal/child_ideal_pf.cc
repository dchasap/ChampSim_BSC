#include "child_ideal_pf.h"

// child_ideal is implemented as a header-only prefetcher so that
// other prefetchers (e.g. pf_combiner) can include and reuse its
// internal tables. This translation unit exists so that the build
// system picks up the directory and can link the prefetcher as a
// standalone module.

// Constructor definition
child_ideal::child_ideal(CACHE* cache) : champsim::modules::prefetcher(cache), offset_bits(0)
{
  // Initialize configuration from env vars
  if (auto e = champsim::EnvVar<int>::get("PF_CHILD_TRAIN_ON_HIT"))
    train_on_hit = (*e != 0);
  else
    train_on_hit = true;

  if (auto e = champsim::EnvVar<int>::get("PF_CHILD_ISSUE_ON_HIT"))
    issue_on_hit = (*e != 0);
  else
    issue_on_hit = false;

  if (auto e = champsim::EnvVar<int>::get("PF_CHILD_FILL_THIS_LEVEL"))
    fill_this_level = (*e != 0);
  else
    fill_this_level = true;

  if (auto e = champsim::EnvVar<int>::get("PF_MSHR_GATE_PCT")) {
    int value = *e;
    mshr_gate_pct = (value > 0 && value <= 100) ? static_cast<uint32_t>(value) : 50u;
  } else {
    mshr_gate_pct = 50;
  }

  std::cout << "Child-Ideal PTE prefetcher: "
            << "train_on_hit=" << (train_on_hit ? 1 : 0)
            << " issue_on_hit=" << (issue_on_hit ? 1 : 0)
            << " fill_this_level=" << (fill_this_level ? 1 : 0)
            << " mshr_gate_pct=" << mshr_gate_pct << std::endl;
}

// Print statistics
void child_ideal::print_stats()
{
  std::cout << "PF TABLES "
            << "TABLE_A_SIZE:" << table_a.size() << " "
            << "TABLE_B_SIZE:" << table_b.size() << std::endl;

  std::cout << "PF OPERATIONS "
            << "ACCESS_COUNT:" << access_count << " "
            << "DISCOVERY_UPDATES:" << discovery_updates << " "
            << "TRAINING_UPDATES:" << training_updates << std::endl;

  std::cout << "PF PREDICTIONS "
            << "LOOKUPS:" << prediction_lookups << " "
            << "HITS:" << prediction_hits << " "
            << "MISSES:" << prediction_misses << " "
            << "HIT_RATE:" << (prediction_lookups > 0 ? (100.0 * static_cast<double>(prediction_hits) / static_cast<double>(prediction_lookups)) : 0.0) << "%" << std::endl;

  std::cout << "PF PREFETCH "
            << "ISSUED:" << prefetches_issued << " "
            << "MSHR_BLOCKED:" << mshr_gate_blocked << std::endl;

  std::cout << "PF UNIQUE COUNTS "
            << "TOTAL_UNIQUE_PTES_OBSERVED:" << unique_ptes.size() << " "
            << "TOTAL_UNIQUE_PARENTS_OBSERVED:" << unique_parents.size() << " "
            << "TOTAL_UNIQUE_CHILDREN_OBSERVED:" << unique_children.size() << " "
            << "TOTAL_UNIQUE_PARENT_CHILD_PAIRS_OBSERVED:" << unique_parent_child_pairs.size() << std::endl;
}
