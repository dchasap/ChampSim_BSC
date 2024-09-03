#include <algorithm>
#include <map>
#include <vector>

#include "cache.h"
#include "predTable.h"
#include "sampler.h"

namespace
{
	// these are for stats, we can remove them
	int nvict = 0;
	int bypass_cnt = 0;
	int samp_dead_victim_cnt = 0;
	int cache_dead_victim_cnt = 0;
	int matched_samp_cnt = 0;
	int empty_sampler_cnt = 0;
	int lru_sampler_cnt = 0;
	int empty_cache_cnt = 0; 
	int cache_samp_dead_cnt = 0;
	// sampler stats, some may be useful
	uint64_t nfalseneg = 0;
	uint64_t nfalsepos = 0;
	uint64_t deadlru = 0;
	uint64_t nevict_samp = 0;
	//uint32_t rep_cnt;
	//uint32_t rep_set_cnt;
	int cache_non_samp_dead_cnt = 0;
	// these are important to compute the signatures
	uint32_t _sampler_set;
	uint32_t _sampler_assoc;
	uint32_t sampler_index_offset;
	uint32_t sampler_blk_offset;
	uint32_t sampler_nblcks;
	uint32_t _sampler_setsize;

	//int group = 0;
	int signature_bits = 16;
	//int dan_sampler_tag_bits = 16;
	// this is important, should be parametrized 
	int threshold_bypass;
	int cache_thresh;
	// this is for predictor table
	int pred_table_index_bits = 16;
	int num_tables = 8;

	cpu_structure module_type;

	// used to update prediction table
	uint32_t last_set;
	
	std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
	std::map<CACHE*, std::vector<bool>> is_dead;
	sampler* _sampler;
	predTable* _predTable;

/*
uint64_t calc_set_index(uint64_t pc)
{
	uint64_t set_index = (pc >> sam_blk_offset ) & ((1 << sam_index_offset) - 1);
	return set_index;
}

uint64_t calc_tag_val(uint64_t pc) 
{
	uint64_t tag_val = pc >> (sam_index_offset + sam_blk_offset);
	return tag_val;
}
*/
using namespace champsim;

inline unsigned int make_signature(uint64_t pc, std::string NAME)
{
	//FIXME: maybe use champsim's
	//uint64_t set_mix = calc_set_index(pc);
	//uint64_t tag_mix =calc_tag_val(pc);
	
	//uint64_t pc_off = pc >> sam_blk_offset;
	//int a = sam_index_offset - group;

	unsigned int mixed = 0;
	if (NAME.compare("cpu0_ITLB") == 0) {
			mixed = (pc) ^ (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	} else if (NAME.compare("cpu0_DTLB") == 0) {
			mixed = (pc) ^ (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	} else if (NAME.compare("cpu0_STLB") == 0) {
			mixed = (pc) ^  (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	}
/*
	if ( way_test == 1010){
		cout <<"PC  :" << PC << "    " << bitset<64>(PC) << endl;
		cout << "signature: " << mixed << bitset<64>(mixed) << endl;
	}
*/
	return (mixed) & ((1 << ::signature_bits) - 1);
}

} // namespace 

void CACHE::initialize_replacement() 
{ 
	::threshold_bypass = 100; // should be parametrized
	::cache_thresh = 1;
	// sampler sizes seem wrong
	::_sampler_set = NUM_SET;
	::_sampler_assoc = NUM_WAY;
	::sampler_index_offset = champsim::msl::lg2(::_sampler_set);
	::sampler_blk_offset = champsim::msl::lg2(PAGE_SIZE); // this is only correct for TLBs
	::sampler_nblcks = (::_sampler_set * ::_sampler_assoc) / PAGE_SIZE;
	::_sampler_setsize = ::_sampler_assoc * PAGE_SIZE;
	// init module_type (only for TLB and caches)
	if (NAME.compare("cpu0_ITLB") == 0) {
		module_type = L1iTLB;
	} else if (NAME.compare("cpu0_DTLB") == 0) {
		module_type = L1dTLB;
	} else if (NAME.compare("cpu0_STLB") == 0) {
		module_type = TLB2;
	} else if (NAME.compare("cpu0_L1I") == 0) {
		module_type = L1icache;
	} else if (NAME.compare("cpu0_L1D") == 0) {
		module_type = L1dcache;
	} else if (NAME.compare("cpu0_L2C") == 0) {
		module_type = L2cache;
	} else if (NAME.compare("LLC") == 0) {
		module_type = L3cache;
	}
	// this is used for LRU
	::last_used_cycles[this] = std::vector<uint64_t>(NUM_SET * NUM_WAY); 
	// here we keep CHiRP predictions
	::is_dead[this] = std::vector<bool>(NUM_SET * NUM_WAY, false); 
	// init sampler
	::_sampler = new sampler(NUM_SET, NUM_WAY, ::_sampler_set, ::_sampler_assoc);
	::_predTable = new predTable(pred_table_index_bits, num_tables);

	std::cout << "CHiRPing..." << std::endl;
}

uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, 
														const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	nvict++;
	uint32_t way = NUM_WAY;
	unsigned int trace = make_signature(ip, NAME);
	// not sure when and why we bypass
	bool prediction_bypass;
	int pred_confindence = ::_predTable->get_prediction(module_type, trace);
	if ( pred_confindence > threshold_bypass ) {
		prediction_bypass = true;
	} else {
		prediction_bypass = false;
	}

	if(prediction_bypass == true ) {
		//r = -2;
		bypass_cnt++;
	} else {
		// check for invalid entreis
		for (unsigned int i = 0; i < NUM_WAY; i++) {
			if (!current_set[i].valid) {
				empty_cache_cnt++;
				way = i;
				break;
			}
		}
		// if no invalid entry was found, look for predicted dead blocks
		if (way == NUM_WAY) {
			for (unsigned int i = 0; i < NUM_WAY; i++) {
				if (is_dead[this].at(set * NUM_WAY + i)) {
					cache_dead_victim_cnt++;
					way = i;
					break;
				}
			}
		}
		// if not found, lookup LRU victim
		if ( way == NUM_WAY) {
  		auto begin = std::next(std::begin(::last_used_cycles[this]), set * NUM_WAY);
  		auto end = std::next(begin, NUM_WAY);

  		// Find the way whose last use cycle is most distant
  		auto victim = std::min_element(begin, end);
  		assert(begin <= victim);
  		assert(victim < end);
  		way = static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by prior asserts
		}
	}
	return way;
}

void CACHE::update_replacement_state(	uint32_t triggering_cpu, uint32_t set, uint32_t way, 
																			uint64_t full_addr, uint64_t ip, uint64_t victim_addr, 
																			uint32_t type, uint8_t hit, REP_POL_XARGS xargs)
{
	unsigned int trace = make_signature(ip, NAME);
	int pred_confidence = ::_predTable->get_prediction(module_type, trace);
	if (pred_confidence >= cache_thresh) {
		is_dead[this].at(set * NUM_WAY + way) = true;
	} else {
		is_dead[this].at(set * NUM_WAY + way) = false;
	}

	if (pred_confidence >= cache_thresh) {
		if(set % ::_sampler->sampler_modulus == 0) {
			cache_samp_dead_cnt++;
		} else {
			cache_non_samp_dead_cnt++;
		}
	}

	// Update Sampler
	int sampler_set = set / ::_sampler->sampler_modulus;
	if (sampler_set < ::_sampler->nsampler_sets) {
	
		sampler_entry *blocks = & (::_sampler->samp_sets)[set].blocks[0];
		uint32_t partial_tag = full_addr;
		bool matchFound = false;
		bool emptyFound = false;
		bool deadFound = false;
		bool feedback = false;
		uint32_t victim = 88; // dummy val
		uint64_t trace_current = make_signature(ip, NAME);
		for (uint32_t i = 0; i < ::_sampler_assoc; i++) {
			if ((blocks[i].valid == true) && (blocks[i].tag == full_addr)) {
				matchFound = true;
				victim = i;
				::matched_samp_cnt++;
				if (blocks[i].prediction == true && warmup == false ) {
					::nfalsepos++;
				}
				feedback = true;
				::_predTable->block_is_dead(module_type, blocks[i].trace, false);
				break;
			}
		}
		if (matchFound == false)
		{
			for (uint32_t i = 0; i < ::_sampler_assoc; i++) {
				if (blocks[i].valid == false) {
					emptyFound = true;
					victim = i;
					::empty_sampler_cnt++;
					break;
				}
			}
			if (emptyFound == false) {
				//int conf_compar = 0 ;
				uint32_t deadest = ::_sampler_assoc;
				for (uint32_t i = 0; i < ::_sampler_assoc; i++) {
					if (blocks[i].prediction == true) {
						deadFound = true;
						victim = i ;
						deadest = i ;
						break;
					}
				}
				if (deadFound == true){
					victim = deadest;
					//myPred->block_is_dead(type, blocks[vict].trace, true, cache_repl); /*disabled for decrease access ratio, activate for higher accuracy
					samp_dead_victim_cnt++;
					feedback = true;
				}
			}

			if (deadFound == false && emptyFound == false) {
				uint32_t j;
				for (j = 0; j < ::_sampler_assoc; j++) {
					if (blocks[j].lru_stack_position == (unsigned int) (::_sampler_assoc-1)) {
						::nevict_samp++;
						if ( blocks[j].prediction == false && warmup == false ){
							::nfalseneg++;
						} else {
							::deadlru++;
						}
						feedback = true;
						::_predTable->block_is_dead(module_type, blocks[j].trace, true);
						::lru_sampler_cnt++;
						break;
					}
				}
				assert(j < ::_sampler_assoc);
				victim = j ;
			}
			blocks[victim].tag = partial_tag;
			blocks[victim].valid = true;
		}
		blocks[victim].trace = trace_current;
		pred_confidence = ::_predTable->get_prediction(type, trace_current);
		if (pred_confidence >= cache_thresh) {
			blocks[victim].prediction = true;
		} else {
			blocks[victim].prediction = false;
		}
		blocks[victim].conf_val = pred_confidence;
		if (feedback == true) {
			//int feedback_conf = pred_confidence;
  
			auto [set_begin, set_end] = get_set_span(full_addr);
  		auto block_it = std::find_if(set_begin, set_end, eq_addr<BLOCK>(full_addr, OFFSET_BITS));

			if (block_it != set_end) {
				if (block_it->valid == true) {
					if (pred_confidence >= cache_thresh) {
						is_dead[this].at(set * ::_sampler_assoc + way) = true;
					} else {
						is_dead[this].at(set * ::_sampler_assoc + way) = false;
					}
				}
			}
		}
		unsigned int position = blocks[victim].lru_stack_position;
		for (unsigned int i=0; i < ::_sampler_assoc; i++)
			if (blocks[i].lru_stack_position < position)
				blocks[i].lru_stack_position++;
		blocks[victim].lru_stack_position = 0;
	}

	// Update prediction Table
	/*
	if (last_pc == ip) {
		rep_cnt++;
	}
	pc_last = ip;
	*/
	if (set == ::last_set) {
		table_update_flag = false;
	}
	else{
		table_update_flag = true;
	}
	::last_set = set;

  // Update LRU
  if (!hit || type != WRITE) // Skip this for writeback hits
    ::last_used_cycles[this].at(set * NUM_WAY + way) = current_cycle;
}

void CACHE::replacement_final_stats() {}
