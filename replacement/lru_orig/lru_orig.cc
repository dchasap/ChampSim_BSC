#include <algorithm>
#include <iterator>
#include <map>
#include <string>

#include "cache.h"
#include "util.h"


/*
 * Note: This is only for cache use (no tlb)
 */

//#define MIN_EVICTION_POSITION 6

namespace {
std::map<CACHE*, std::vector<uint32_t>> least_recently_used;
}

void CACHE::initialize_replacement() {
	::least_recently_used[this] = std::vector<uint32_t>(NUM_SET * NUM_WAY); 
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	uint32_t lru_victim_index = 0;
	for (uint32_t i = 0; i < NUM_WAY; i++) {
		
		//std::cout << "lru:" << least_recently_used[this][set * NUM_WAY + i] << std::endl;
		uint32_t max_lru = 0;
		if (max_lru < ::least_recently_used[this][set * NUM_WAY + i]) {
			lru_victim_index = i;
			max_lru = ::least_recently_used[this][set * NUM_WAY + i];
		}
	}

//	std::cout << "victim:" << lru_victim_index << std::endl;
	return lru_victim_index;
	//return 0;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
	if (!hit || type != WRITE) 
		return;

	//std::cout << "that's not good" << std::endl;
  uint32_t hit_lru = ::least_recently_used[this][set * NUM_WAY + way];
//	std::cout << "=================================" << std::endl;
//	std::cout << "way:" << way << " - hit_lru:" << hit_lru << std::endl;
//	std::cout << "orig:" << std::endl;
  for (uint32_t i = 0; i < NUM_WAY; i++) {
//		std::cout << "lru[" << i  << "]=" << ::least_recently_used[this][set * NUM_WAY + i] << std::endl;
    if (::least_recently_used[this][set * NUM_WAY + i] <= hit_lru) {
    	::least_recently_used[this][set * NUM_WAY + i] = ::least_recently_used[this][set * NUM_WAY + i] + 1;
		}
  }
  ::least_recently_used[this][set * NUM_WAY + way] = 0; // promote to the MRU position

//	std::cout << "new:" << std::endl;
//  for (uint32_t i = 0; i < NUM_WAY; i++) {
//		std::cout << "lru[" << i  << "]=" << ::least_recently_used[this][set * NUM_WAY + i] << std::endl;
//	}

	return;
}

void CACHE::replacement_final_stats() {}

