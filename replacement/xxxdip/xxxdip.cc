#include <algorithm>
#include <map>
#include <utility>
#include <string>

#include "cache.h"
#include "util.h"

//#include "xxxdip.h"
uint32_t instr_pos, data_pos;
uint32_t maxRRPV;

typedef struct _lru_entry {
	uint32_t lru;
	bool is_instr;
} lru_entry;

std::map<uint64_t, int32_t> vpn_freq_acc;
std::map<CACHE*, std::vector<lru_entry>> least_recently_used;
//std::map<CACHE*, uint32_t> least_recently_used;

void CACHE::initialize_replacement()
{
	least_recently_used[this] = std::vector<lru_entry>(NUM_SET * NUM_WAY);
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{
	if (!type) {
		// cache hit
		if (hit) {
			least_recently_used[this][set * NUM_WAY + way].lru = maxRRPV - data_pos; // for cache hit, DRRIP always promotes	
			least_recently_used[this][set * NUM_WAY + way].is_instr = false;
			return;
		}

		least_recently_used[this][set * NUM_WAY + way].lru = maxRRPV - 1;
		least_recently_used[this][set * NUM_WAY + way].is_instr = false;
		return;
	}

	/*
	 * idip_fac
	 */
	//uint64_t vpn = block[set * NUM_WAY + way].v_address >> LOG2_PAGE_SIZE;
	uint64_t vpn = victim_addr >> LOG2_PAGE_SIZE;
	// get access frequency and setup new entry if it doesn't exit
	int32_t acc_freq = 0;
	if (vpn_freq_acc.find(vpn) == vpn_freq_acc.end()) {
		acc_freq = -2;
		vpn_freq_acc[vpn] = -2;
	} else {
		acc_freq = vpn_freq_acc[vpn];
	}

	// choose the correct placement for new block/translation
	if (acc_freq < 50) {
		least_recently_used[this][set * NUM_WAY + way].lru = maxRRPV - 1;
		least_recently_used[this][set * NUM_WAY + way].is_instr = true;
	}
	else {
		least_recently_used[this][set * NUM_WAY + way].lru = 0;
		least_recently_used[this][set * NUM_WAY + way].is_instr = true;
		//std::cout << maxRRPV - ((acc_freq / 50) % 4) << std::endl;
		//block[set * NUM_WAY + way].lru = maxRRPV - ((acc_freq / 50) % 4);
	}

	// update fac
	vpn_freq_acc[vpn]++;

	return;

}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  // look for the maxRRPV line
  auto begin = std::next(std::begin(least_recently_used[this]), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);
  auto victim = std::find_if(begin, end, [](lru_entry x) { return !x.is_instr; });
	// check if there is any data block
	if (victim != end) {
  	victim = std::max_element(begin, end, [](lru_entry a, lru_entry b) { return a.lru < b.lru; });
		return std::distance(begin, victim);
	}
	
  victim = std::find_if(begin, end, [](lru_entry x) { return x.lru == maxRRPV; }); 

  while (victim == end) {
  	for (auto it = begin; it != end; ++it)
      it->lru++;

  	victim = std::find_if(begin, end, [](lru_entry x) { return x.lru == maxRRPV; });
  }

  return std::distance(begin, victim);
}
/*
bool cmp(const std::pair<uint64_t, uint32_t> &a, const std::pair<uint64_t, uint32_t> &b) 
{
	return (a.second < b.second);
}
*/

// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {}



