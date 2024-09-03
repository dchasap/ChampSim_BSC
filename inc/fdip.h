#include <queue>
#include <map>
#include <assert.h>
#include "instruction.h"

class FDIP {
  private:
    std::deque<ooo_model_instr> BUFFER;
    const uint32_t width;
    const uint32_t offset;
    const uint32_t aggressivity;
    const uint32_t prune_factor;
    uint64_t last_added_instr_id; 
    bool enabled;

    // stats
    std::map<uint64_t, uint64_t> inflight_mops;
    uint64_t mops;
    uint64_t cycles;

  public:
    FDIP(uint64_t agg) : width(6), offset(10*width), aggressivity(agg), prune_factor(0), enabled (agg > 0) {
        last_added_instr_id = 0;
        mops = 0;
        cycles = 0;
        std::cout << "FDiP ";
        if (enabled) std::cout << "enabled with aggresivity " << aggressivity;
        else std::cout << "disabled";
        std::cout << std::endl;
    };
    ~FDIP() {};

    uint32_t    getAggresivity()    { return aggressivity;}
    uint32_t    getOffset()         { return offset;}
    uint32_t    getWidth()          { return width;}
    auto        getLastAddedInstr() { return last_added_instr_id;}
    bool        empty()             { return BUFFER.empty();}
    bool        isEnabled()         { return enabled; }

    /*
     * Prune the top of the BUFFER to avoid too close 
     * prefetches
     */
    void prune() {
        for (uint32_t i = 0; i < prune_factor; ++i){
            if (empty()) return;
            BUFFER.pop_front();
        }
    }

    void push_back(std::deque<ooo_model_instr>::iterator instr) {
        last_added_instr_id = instr->instr_id;
        // filter same block instr
        uint64_t ip = instr->ip;
        if (std::find_if(   std::begin(BUFFER), 
                            std::end(BUFFER), 
                            [ip] (auto x) { 
                                return ((ip >> LOG2_BLOCK_SIZE) == (x.ip >> LOG2_BLOCK_SIZE));
                            }) == std::end(BUFFER)){
            BUFFER.push_back(*instr);
        }
    }

    std::pair<uint64_t, uint64_t> get_prefetch_line() {
        assert(!empty());
        uint64_t ip = BUFFER.front().ip;
        uint64_t instr_id = BUFFER.front().instr_id;
        BUFFER.pop_front();
        return {ip, instr_id};
    }
    
    void issue_prefetch(uint64_t instr_id, uint64_t event_cycle) {
         inflight_mops[instr_id]= event_cycle;
    }

    void issue_instruction(uint64_t instr_id, uint64_t event_cycle){
        if (inflight_mops.find(instr_id) != inflight_mops.end()){
            uint64_t consumed_cycles = event_cycle - inflight_mops[instr_id];
            mops ++;
            cycles += consumed_cycles;
        }
    }

    void print_stats() {
        double delay = 0.0;
        if (mops > 0){
            delay = (double) cycles / (double) mops;
        }
        std::cout << "[FDIP] Average delay: " << delay << std::endl;
    }

};
