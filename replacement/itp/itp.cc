#include "itp.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <string>
#include <cstdlib>

#include "cache.h"
#include "env_var.h"

uint32_t instr_pos, data_pos;
uint32_t maxRRPV;

std::map<uint64_t, int32_t> vpn_freq_acc;

namespace {
	class SatCnt {
		private:
			uint32_t cnt;
			uint32_t max_value;

		public:
			SatCnt(uint32_t nbits) {
				max_value = 1 << nbits;
				max_value = 50;
				std::cout << "Init sat counter with " << nbits << " bits (max:" << max_value  << ")." << std::endl;
			}

			SatCnt operator++(int) {
				SatCnt tmp = *this;
    		if (cnt < max_value) cnt++;
				return tmp;
			}

			void reset() {
				cnt = 0;
			}

			bool saturated() {
				return (cnt == max_value);
			}
	};

	uint32_t TLB_LOWER_STRESS_THRESHOLD;
	uint32_t TLB_UPPER_STRESS_THRESHOLD;
	std::map<CACHE*, std::vector<uint64_t>> last_used_cycles;
	std::map<CACHE*, std::vector<uint32_t>> least_recently_used;
	std::map<CACHE*, std::vector<SatCnt>> freq_cnt;
}

itp::itp(CACHE* cache) : itp(cache, cache->NUM_SET, cache->NUM_WAY) {}

itp::itp(CACHE* cache, long sets_, long ways_) : replacement(cache) {
    (void)sets_; // suppress unused warning
    (void)ways_; // suppress unused warning
}

void itp::initialize_replacement() {
    if (auto v = champsim::EnvVar<int>::get("TLB_LOWER_STRESS_THRESHOLD")) {
        ::TLB_LOWER_STRESS_THRESHOLD = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("TLB_UPPER_STRESS_THRESHOLD")) {
        ::TLB_UPPER_STRESS_THRESHOLD = *v;
    }

    if (auto v = champsim::EnvVar<int>::get("ITP_MAX_LRU")) {
        maxRRPV = *v;
    } else {
        std::cerr << "ITP_MAX_LRU not set!" << std::endl;
        exit(1);
    }
    if (auto v = champsim::EnvVar<int>::get("ITP_INSTR_POS")) {
        instr_pos = *v;
    } else {
        std::cerr << "ITP_INSTR_POS not set!" << std::endl;
        exit(1);
    }
    if (auto v = champsim::EnvVar<int>::get("ITP_DATA_POS")) {
        data_pos = *v;
    } else {
        std::cerr << "ITP_DATA_POS not set!" << std::endl;
        exit(1);
    }
    
    std::cout << intern_->NAME << " is using iTP" << std::endl;
    std::cout << "\tMAX_RRPV:" << maxRRPV << std::endl;
    std::cout << "\tINSTR_POS:" << instr_pos << std::endl;
    std::cout << "\tDATA_POS:" << data_pos << std::endl;

    ::last_used_cycles[intern_] = std::vector<uint64_t>(intern_->NUM_SET * intern_->NUM_WAY);
    ::least_recently_used[intern_] = std::vector<uint32_t>(intern_->NUM_SET * intern_->NUM_WAY);
    ::freq_cnt[intern_] = std::vector<SatCnt>(intern_->NUM_SET * intern_->NUM_WAY, SatCnt(3));
}

// ITP requires the EXPAND_PACKET macro to be defined so that is_instr, is_pte, and
// translation_level are available to the replacement policy. Without it, the policy
// cannot correctly distinguish instruction/PTE accesses, so we refuse to compile.
static_assert(true, "");

#if !defined(EXPAND_PACKET)
#error "ITP replacement policy requires the build to define EXPAND_PACKET. \
Add -DEXPAND_PACKET to your CXXFLAGS or build configuration."
#endif

// called on every cache hit and cache fill
void itp::update_replacement_state(uint32_t triggering_cpu, long set, long way,
                                    champsim::address full_addr, champsim::address ip,
                                    champsim::address victim_addr, access_type type, uint8_t hit
#if defined(EXPAND_PACKET)
                                    ,
                                    CACHE::repl_pol_xargs xargs
#endif
                                    ) {
    (void)triggering_cpu;
    (void)full_addr;
    (void)ip;

#if defined(EXPAND_PACKET)
    // Extract is_instr from xargs
    bool is_instr = xargs.is_instr;

    if (!is_instr) {
        // cache hit
        if (hit) {
            ::least_recently_used[intern_][set * intern_->NUM_WAY + way] = maxRRPV - data_pos;
            return;
        }

        ::least_recently_used[intern_][set * intern_->NUM_WAY + way] = maxRRPV - 1;
        return;
    }

    // Instruction access path
    uint64_t vpn = victim_addr.to<uint64_t>() >> LOG2_PAGE_SIZE;
    // get access frequency and setup new entry if it doesn't exist
    int32_t acc_freq = 0;
    if (vpn_freq_acc.find(vpn) == vpn_freq_acc.end()) {
        acc_freq = -2;
        vpn_freq_acc[vpn] = -2;
    } else {
        acc_freq = vpn_freq_acc[vpn];
    }

    // choose the correct placement for new block/translation
    if (acc_freq < 50) {
        ::least_recently_used[intern_][set * intern_->NUM_WAY + way] = maxRRPV - instr_pos;
    } else {
        ::least_recently_used[intern_][set * intern_->NUM_WAY + way] = 0;
    }

    // update fac
    vpn_freq_acc[vpn]++;

    if (!hit) {
        ::freq_cnt[intern_][set * intern_->NUM_WAY + way].reset();
    } else {
        ::freq_cnt[intern_][set * intern_->NUM_WAY + way]++;
    }
#endif // EXPAND_PACKET
}

// find replacement victim
long itp::find_victim(uint32_t triggering_cpu, uint64_t instr_id, long set,
                       const champsim::cache_block* current_set, champsim::address ip,
                       champsim::address full_addr, access_type type
#if defined(EXPAND_PACKET)
                       ,
                       CACHE::repl_pol_xargs xargs
#endif
                       ) {
    (void)triggering_cpu;
    (void)instr_id;
    (void)current_set;
    (void)ip;
    (void)full_addr;
    (void)type;
#if defined(EXPAND_PACKET)
    (void)xargs;
#endif
    
    // Look for the maxRRPV line
    auto begin = std::next(std::begin(::least_recently_used[intern_]), set * intern_->NUM_WAY);
    auto end = std::next(begin, intern_->NUM_WAY);
    
    auto victim = std::find_if(begin, end, [this](uint32_t x) { return x == maxRRPV; });
    
    while (victim == end) {
        for (auto it = begin; it != end; ++it)
            (*it)++;

        victim = std::find_if(begin, end, [this](uint32_t x) { return x == maxRRPV; });
    }

    return std::distance(begin, victim);
}

void itp::replacement_cache_fill(uint32_t triggering_cpu, long set, long way,
                                 champsim::address full_addr, champsim::address ip,
                                 champsim::address victim_addr, access_type type
#if defined(EXPAND_PACKET)
                                 ,
                                 CACHE::repl_pol_xargs xargs
#endif
                                 ) {
    // ITP doesn't have special handling for cache fills
    // Just update the replacement state
#if defined(EXPAND_PACKET)
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, true, xargs);
#else
    update_replacement_state(triggering_cpu, set, way, full_addr, ip, victim_addr, type, true);
#endif
}

void itp::replacement_final_stats() {
    // Print VPN frequency stats if needed
    // The original code printed sorted frequency data
}

itp_set_helper::itp_set_helper(long ways) : rrpv_values(static_cast<std::size_t>(ways), maxRRPV) {}

auto itp_set_helper::get_rrpv(long way) -> rrpv_type& { 
    return rrpv_values.at(static_cast<std::size_t>(way)); 
}

long itp_set_helper::victim() {
    // Find the maximum RRPV
    auto victim = std::max_element(std::begin(rrpv_values), std::end(rrpv_values));

    // If the maximum element has RRPV less than the maximum, increment everything to the maximum
    std::transform(std::cbegin(rrpv_values), std::cend(rrpv_values), std::begin(rrpv_values), 
                   [diff = maxRRPV - *victim](auto x) { return x + diff; });

    // Return the way index
    return std::distance(std::begin(rrpv_values), victim);
}

void itp_set_helper::update(long way, bool hit, bool is_instr, uint32_t pos) {
    (void)way;
    (void)hit;
    (void)is_instr;
    (void)pos;
    // ITP uses a different update mechanism - this is a placeholder
    // The actual update logic is in itp::update_replacement_state
}