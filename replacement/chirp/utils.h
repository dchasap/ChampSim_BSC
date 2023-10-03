
/*
void cache::calc(uint64_t pc, uint64_t& set_index, uint64_t& tag_val)
{
	set_index = (PC >> sam_blk_offset ) & ((1 << sam_index_offset) - 1);
	tag_val = PC >> (sam_index_offset + sam_blk_offset);
}
*/

uint64_t calc_set_index(uint64_t pc)
{
	uint64_t set_index = (PC >> sam_blk_offset ) & ((1 << sam_index_offset) - 1);
	return set_index;
}

uint64_t calc_set_index(uint64_t pc) 
{
	uint64_t tag_val = PC >> (sam_index_offset + sam_blk_offset);
	return tag_val;
}

unsigned int make_signature(uint64_t pc)
{
	uint64_t set_mix = calc_set_index(pc);
	uint64_t tag_mix =calc_tag_val(pc);
	
	uint64_t pc_off = pc >> sam_blk_offset;
	int a = sam_index_offset - group;

	if (NAME.compare("cpu0_ITLB") == 0) {
			mixed = (pc) ^ (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	} else if (NAME.compare("cpu0_DTLB") == 0) {
			mixed = (c) ^ (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	} else if (NAME.compare("cpu0_STLB") == 0) {
			mixed = (PC) ^  (condHistory_old) ^ (uncondIndHistory_old) ^ (global_path_history_MHRP);
	}
/*
	if ( way_test == 1010){
		cout <<"PC  :" << PC << "    " << bitset<64>(PC) << endl;
		cout << "signature: " << mixed << bitset<64>(mixed) << endl;
	}
*/
	return (mixed) & ((1 << signature_bits) - 1);
}


