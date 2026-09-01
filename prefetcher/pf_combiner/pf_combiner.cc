#include "pf_combiner.h"

// pf_combiner is implemented as a header-only prefetcher so that
// it can include child_ideal's internals for combining the PTE
// prefetcher with a regular prefetcher.
// This translation unit exists so that the build system picks up
// the directory and can link the prefetcher as a standalone module.
