#include <algorithm>
#include <map>
#include <utility>
#include <string>

#include "cache.h"
#include "util.h"
#include "memory_class.h"

namespace
{
std::map<CACHE*, std::vector<uint32_t>> least_recently_used;
std::map<CACHE*, std::vector<uint32_t>> is_instr;
//uint32_t instr_pos, data_pos, MRU;
uint32_t MRU;
}


std::map<uint64_t, int32_t> vpn_freq_acc;

void CACHE::initialize_replacement()
{
/*
	::MRU = std::stoi(getenv("XDIP_MAX_LRU"));
	::instr_pos = std::stoi(getenv("XDIP_INSTR_POS"));
	::data_pos = std::stoi(getenv("XDIP_DATA_POS"));
	std::cout << this->NAME << " using xdim with max lru@" <<:: maxRRPV << std::endl;
	std::cout << this->NAME << " using xdim with instr@" << ::instr_pos 
						<< " and data@" << ::data_pos << std::endl;
*/
	::MRU = NUM_WAY; 
	::least_recently_used[this] = std::vector<uint32_t>(NUM_SET * NUM_WAY);
	::is_instr[this] = std::vector<uint32_t>(NUM_SET * NUM_WAY);
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type, uint8_t hit)
{

	// FIXME: we don't care for other types,  so we hijack type variable for passing is_instr
	if (type) { // if it's instr
		::is_instr[this][way] = true;
  	uint32_t hit_lru = ::least_recently_used[this][way];
		for (unsigned int i = 0; i < NUM_WAY; i++) {
  		if (::least_recently_used[this][i] > hit_lru) {
      	::least_recently_used[this][i]--;
      	//assert(::least_recently_used[this][i] != 0);
			}
  	}
  	::least_recently_used[this][way] = MRU; // promote to the MRU position
	} else { // data block
		// find the MRU of data blocks
		uint32_t data_mru = 0;
		for (unsigned int i = 0; i < NUM_WAY; i++) {
			if (!::is_instr[this][i])
				data_mru = ::least_recently_used[this][i];
		}
		//data_mru = data_block_count;
  	uint32_t hit_lru = ::least_recently_used[this][way];
		for (unsigned int i = 0; i < NUM_WAY; i++) {
			// only move blocks at the lower half of the stack
  		if (::least_recently_used[this][i] > hit_lru && ::least_recently_used[this][i] <= data_mru) {
      	::least_recently_used[this][i]--; // this can potentially also move instr blocks
				//assert(*it != 0);
			}
  	}
		::is_instr[this][way] = false;
  	::least_recently_used[this][way] = data_mru; // promote to the MRU position
	}
	/*
	 * idip_fac
	 */
/*
	//FIXME: is this the virtual or physical address
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
		block[set * NUM_WAY + way].lru = maxRRPV - 1;
	}
	else {
		block[set * NUM_WAY + way].lru = 0;
		//std::cout << maxRRPV - ((acc_freq / 50) % 4) << std::endl;
		//block[set * NUM_WAY + way].lru = maxRRPV - ((acc_freq / 50) % 4);
	}

	// update fac
	vpn_freq_acc[vpn]++;
*/
	return;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	uint32_t victim = 0;
	for (uint32_t i = 0; i < NUM_WAY; i++) {
		if (::least_recently_used[this][i] == 0) {
			victim = i;
			break;
		}
	}

	return victim;
}


// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {
/*
	std::vector <pair<uint64_t, uint32_t>> sorted_fac;
	for (auto& i : vpn_freq_acc) {
		sorted_fac.push_back(i);
	}	
	std::sort(sorted_fac.begin(), sorted_fac.end(), cmp);
	for (auto& i : sorted_fac) {
		std::cout << std::hex << i.first << std::dec;
		std::cout << ", " << i.second + maxRRPV + 1 << std::endl;
	}
	*/
}



