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

	typedef struct _eviction_entry 
	{
		uint32_t lru_value;
		bool is_pte;
		bool is_data;

		_eviction_entry() : lru_value(0), is_pte(false), is_data(false) {}
	} eviction_entry;

	std::map<CACHE*, std::vector<eviction_entry>> least_recently_used;
	uint32_t MIN_EVICTION_POSITION; 
}

void CACHE::initialize_replacement() 
{
	::MIN_EVICTION_POSITION = std::stoi(getenv("MIN_EVICTION_POSITION"));
	std::cout << NAME << " is using LRU_PTP" << std::endl; 
	std::cout << "\tMIN_EVICTION_POSITION" << ::MIN_EVICTION_POSITION << std::endl;

	::least_recently_used[this] = std::vector<eviction_entry>(NUM_SET * NUM_WAY); 
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	uint32_t lru_victim_index = 0;
	uint32_t alt_victim_index = 0, max_alt_lru = 0;
	for (uint32_t i = 0; i < NUM_WAY; i++) {
		//std::cout << "lru:" << least_recently_used[this][set * NUM_WAY + i].lru << std::endl;
		if (::least_recently_used[this][set * NUM_WAY + i].lru_value == (NUM_WAY-1)) {
			lru_victim_index = i;
		}
		
		if (::least_recently_used[this][set * NUM_WAY + i].is_pte 
				&& ::least_recently_used[this][set * NUM_WAY + i].is_data 
				&& ::least_recently_used[this][set * NUM_WAY + i].lru_value > max_alt_lru) {

			max_alt_lru = ::least_recently_used[this][set * NUM_WAY + i].lru_value;
			alt_victim_index = i;

		}
		
	}

	if (max_alt_lru >= MIN_EVICTION_POSITION) {
		// adjust lru value to entries that should have been evicted instead
		for (uint32_t i = 0; i < NUM_WAY; i++) {
    	if (::least_recently_used[this][set * NUM_WAY + i].lru_value > max_alt_lru) {
    		::least_recently_used[this][set * NUM_WAY + i].lru_value--;
			}
  	}

		return alt_victim_index;
	}

	return lru_victim_index;
	//return MIN_EVICTION_POSITION;
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
/*
	if (!hit || type != WRITE) 
		return;
*/
  uint32_t hit_lru = ::least_recently_used[this][set * NUM_WAY + way].lru_value;

  for (uint32_t i = 0; i < NUM_WAY; i++) {
    if (::least_recently_used[this][set * NUM_WAY + i].lru_value <= hit_lru) {
    	::least_recently_used[this][set * NUM_WAY + i].lru_value++;
		}
  }
  ::least_recently_used[this][set * NUM_WAY + way].lru_value = 0; // promote to the MRU position
	::least_recently_used[this][set * NUM_SET + way].is_pte = static_cast<bool>(hit);
	::least_recently_used[this][set * NUM_SET + way].is_data = !static_cast<bool>(type);
	return;
}

void CACHE::replacement_final_stats() {}

