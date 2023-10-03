#include <algorithm>
#include <map>
#include <vector>

#include "cache.h"

namespace
{
std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
std::map<CACHE*, std::vector<uint32_t>> access_frequency;
std::map<CACHE*, std::vector<uint64_t>> way_evicted;
uint64_t times_used_lfu;
uint64_t times_used_lru;
}

void CACHE::initialize_replacement() { 
	::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY);
	::access_frequency[this] = std::vector<uint32_t>(NUM_SET * NUM_WAY, 0); 
	::way_evicted[this] = std::vector<uint64_t>(NUM_WAY, 0); 
	times_used_lfu = 0;
	times_used_lru = 0;
}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	uint32_t min_freq = ::access_frequency[this].at(set * NUM_WAY);
	// they should actually take their value at first iter, but compiler complains otherwise
	uint64_t victim = 0, lfu_victim = 0;
	uint64_t last_lru = ::last_used_cycles[this].at(set * NUM_WAY);
	for (unsigned int i = 1; i < NUM_WAY; i ++) {
		if (::access_frequency[this].at(set * NUM_WAY + i) < min_freq) {
			min_freq = ::access_frequency[this].at(set * NUM_WAY + i);
			last_lru = ::last_used_cycles[this].at(set * NUM_WAY + i);
			victim = i;
			lfu_victim = i;
		} else if (::access_frequency[this].at(set * NUM_WAY + i) == min_freq 
							&& ::last_used_cycles[this].at(set * NUM_WAY + i) < last_lru) {	
			//std::cout << "choosing LRU" << std::endl;
			last_lru = ::last_used_cycles[this].at(set * NUM_WAY + i);
			victim = i;
		}
	}
	//std::cout << "victim:" << victim << std::endl;
	//
	::way_evicted[this].at(victim)++;
	if (victim == lfu_victim) times_used_lfu++;
	else times_used_lru++;
	return victim;
}

void CACHE::update_replacement_state(	uint32_t triggering_cpu, uint32_t set, uint32_t way, 
																			uint64_t full_addr, uint64_t ip, uint64_t victim_addr, 
																			uint32_t type, uint8_t hit, REP_POL_XARGS xargs)
{
  // Mark the way as being used on the current cycle
  if (!hit || type != WRITE)  {// Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;

		if (hit)
    	::access_frequency[this].at(set * NUM_WAY + way)++;
		else
    	::access_frequency[this].at(set * NUM_WAY + way) = 0;
	}
}

void CACHE::replacement_final_stats() 
{
	std::cout << "Printing extra LFU statistics." << std::endl;
	for (unsigned int i = 0; i < NUM_WAY; i++) {
		std::cout << "\tWAY" << i << ":" << ::way_evicted[this].at(i) << std::endl;	
	}
	std::cout << "Times evicted with LFU:" << times_used_lfu << std::endl;
	std::cout << "Times evicted with LRU:" << times_used_lru << std::endl;
}

