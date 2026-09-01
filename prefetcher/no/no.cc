#include "no.h"

champsim::modules::pf_meta_t no::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                      champsim::modules::pf_meta_t metadata_in)
{
  (void)addr; (void)ip; (void)cache_hit; (void)useful_prefetch; (void)type;
  return metadata_in;
}

champsim::modules::pf_meta_t no::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in)
{
  (void)addr; (void)set; (void)way; (void)prefetch; (void)evicted_addr;
  return metadata_in;
}
