#pragma once

#include <barrier>

#include "core/thread_pool.hpp"
#include "shared/morton_func.h"

namespace cpu {

// ---------------------------------------------------------------------
// Radix Sort (challenging)
// ---------------------------------------------------------------------

[[nodiscard]] core::multi_future<void> dispatch_binning_pass(
    core::thread_pool& pool,
    const size_t n_threads,
    std::barrier<>& barrier,
    const int n,
    const morton_t* u_sort,
    morton_t* u_sort_alt,
    const int shift);

}  // namespace cpu