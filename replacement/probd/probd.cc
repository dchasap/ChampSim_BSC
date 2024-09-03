#include <algorithm>
#include <map>
#include <utility>
#include <string>

#include "cache.h"
#include "util.h"

namespace {

	struct LRUStackElem {
		uint64_t last_used_cycle;
		bool is_instr;
	}

	std::map<CACHE*, std::vector<LRUStackElem>> last_used_cycles;
	uint32_t data_eviction_prob;
}


void CACHE::initialize_replacement()
{
	data_eviction_prob = std::stoi(getenv("PROBR_EVICT_PROB"));

	std::cout << this->NAME << " using iTP with max lru@" << maxRRPV << std::endl;
	std::cout << this->NAME << " using iTP with instr@" << instr_pos 
						<< " and data@" << data_pos << std::endl;
		
	std::cout << this->NAME << " using probabilistic eviction with data eviction probability " 
						<< instr_eviction_prob << "%" << std::endl;

	::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 

  srand((unsigned) time(NULL));
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(	uint32_t triggering_cpu, uint32_t set, uint32_t way, 
																			uint64_t full_addr, uint64_t ip, uint64_t victim_addr, 
																			uint32_t type, uint8_t hit,
																			CACHE::REP_POL_XARGS xargs)
{
  // Mark the way as being used on the current cycle
  if (!hit || type != WRITE) { // Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way).last_used_cycle = current_cycle;
    ::last_used_cycles[this].at(set * NUM_WAY + way).is_instr = xargs.is_instr;
	}

	return;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	uint64_t probability = rand() % 100;
	if (probability < data_eviction_prob) {
  	
		auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
  	auto end = std::next(begin, NUM_WAY);
		uint32_t max_cycle = 0;
		bool found_victim = false;
		for (auto it = begin; it != end; ++it) {
			if (max_cycle >= && it->is_instr) {
				found_victim = true;
				max_cycle = it->last_used_cycle;
			}
		}

		if (found)
  		return static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by prior asserts
	}
  
	begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
 	end = std::next(begin, NUM_WAY);

  // Find the way whose last use cycle is most distant
  victim = std::min_element(begin, end);
  return static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by prior asserts

}


// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {}

