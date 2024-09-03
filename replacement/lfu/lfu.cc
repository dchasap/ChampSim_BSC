#include <algorithm>
#include <map>
#include <vector>

#include "cache.h"

namespace
{
  std::map<CACHE*, std::vector<uint64_t>> freq_ctr;
  // std::vector<uint64_t> hit_position;
}

uint32_t get_lfu_hit_position(std::vector<uint64_t>::iterator begin,std::vector<uint64_t>::iterator end, uint64_t penalty){
  std::vector<uint64_t> tem_vec(begin,end);
  std::sort(tem_vec.begin(), tem_vec.end(), [](uint64_t a, uint64_t b) { return a > b; });
  auto order = std::find(tem_vec.begin(), tem_vec.end(), penalty);
  assert(order != tem_vec.end());
  return static_cast<uint32_t>(std::distance(tem_vec.begin(), order));
}

void CACHE::initialize_replacement() { 
  std::cout << NAME << " LFU " << " SETS: " << NUM_SET << " WAYS: " << NUM_WAY << " SIZE: " << NUM_SET * NUM_WAY * 64 / 1024 << "KB" << std::endl;
  ::freq_ctr[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY, 0); 
  // hit_position = std::vector<uint64_t>(NUM_WAY, 0); 
}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
  auto begin = std::next(std::begin(::freq_ctr[this]), set * NUM_WAY);
  auto end = std::next(begin, NUM_WAY);

  // Find the way whose last use frequency cntr has the lowest value
  auto victim = std::min_element(begin, end);
  assert(begin <= victim);
  assert(victim < end);
  uint32_t victim_idx = static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by prior asserts
	//std::cout << "victim:" << victim_idx << std::endl;
	return victim_idx;
}

void CACHE::update_replacement_state(	uint32_t triggering_cpu, uint32_t set, uint32_t way, 
																			uint64_t full_addr, uint64_t ip, uint64_t victim_addr, 
																			uint32_t type, uint8_t hit, REP_POL_XARGS xargs)
{
  // Mark the way as being used on the current cycle
  if (hit && type == WRITE) { // Skip this for writeback hits
    // auto begin = std::next(std::begin(::freq_ctr[this]), set * NUM_WAY);
    // auto end = std::next(begin, NUM_WAY);
    // if (hit) 
    //   hit_position[get_lfu_hit_position(begin, end,::freq_ctr[this].at(set * NUM_WAY + way))]++;
		return;
  }
  if (hit) ::freq_ctr[this].at(set * NUM_WAY + way) ++;
  else ::freq_ctr[this].at(set * NUM_WAY + way) = 0;
}

void CACHE::replacement_final_stats() {
  // float total = 0.0;
  // float accum = 0.0;
  // std::cout << NAME << " LFU:";
  // for (uint32_t i = 0; i < hit_position.size(); i++) {
  //   total += hit_position[i];
  //   std::cout << std::setw(5) << i;
  // }
  // std::cout << std::endl;

  // std::cout << std::setw(15);
  // for (auto x : hit_position ) {
  //   accum += x;
  //   std::cout << std::setw(5) << std::setprecision(2) << (accum/total);
  // }
  // std::cout << std::endl;
}
