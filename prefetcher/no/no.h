#ifndef PREFETCHER_NO_H
#define PREFETCHER_NO_H

#include <cstdint>

#include "champsim.h"
#include "modules.h"

class no : public champsim::modules::prefetcher
{
public:
  using prefetcher::prefetcher;
  champsim::modules::pf_meta_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, bool useful_prefetch, access_type type,
                                    champsim::modules::pf_meta_t metadata_in);
  champsim::modules::pf_meta_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, champsim::address evicted_addr, champsim::modules::pf_meta_t metadata_in);
};

#endif
