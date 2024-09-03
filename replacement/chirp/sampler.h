
#ifndef SAMPLER_H
#define SAMPLER_H

#include "predTable.h"

int sampler_assoc;

struct sampler_entry {
    unsigned int lru_stack_position;
 		unsigned int tag;
    unsigned int trace;
    int conf_val;
    bool valid;
    bool prediction;
    
		sampler_entry(void) 
		{
        lru_stack_position = 0;
        conf_val = 0 ;
        valid = false;
        tag = 0;
        trace = 0;
        prediction = false;
    };
};

struct sampler_set {
    sampler_entry *blocks;
    sampler_set();
    ~sampler_set();
};

sampler_set::sampler_set() 
{
    blocks = new sampler_entry[sampler_assoc];
    for (int i=0; i < sampler_assoc; i++) {
        blocks[i].lru_stack_position = i;
        blocks[i].tag = 0;
        blocks[i].trace = 0 ;
        blocks[i].conf_val = 0 ;
        blocks[i].valid = false;
        blocks[i].prediction = false;
    }
}

sampler_set::~sampler_set(){
        delete [] blocks;
}

struct sampler {
    sampler_set *samp_sets;
    int nsampler_sets;
    int sampler_modulus;
    ~sampler();
    sampler(int nsets,int assoc, uint32_t sampl_sets, int dan_sampler_assoc);
};

sampler::sampler(int nsets, int assoc, uint32_t sampl_sets, int dan_sampler_assoc)
{
 		sampler_assoc = dan_sampler_assoc;
    nsampler_sets = sampl_sets;
    samp_sets = new sampler_set[nsampler_sets];
    sampler_modulus = nsets / nsampler_sets;
}

sampler::~sampler()
{
    delete [] samp_sets;
}

#endif
