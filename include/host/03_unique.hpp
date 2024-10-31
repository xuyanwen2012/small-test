#pragma once

#include <future>

#include "core/thread_pool.hpp"
#include "shared/morton_func.h"

namespace cpu {

// ---------------------------------------------------------------------
// Unique (for CPU, should only use a single thread), we have small problem size
//
// And this future will return the number of unique elements
//
// ---------------------------------------------------------------------

// Custom unique_copy implementation

[[nodiscard]] inline std::future<int> dispatch_unique(core::thread_pool& pool,
                                                      const int n,
                                                      const morton_t* u_morton,
                                                      morton_t* u_morton_alt) {
  return pool.submit_task([=, &u_morton, &u_morton_alt]() {
    const auto last = std::unique_copy(u_morton, u_morton + n, u_morton_alt);
    const auto n_unique = std::distance(u_morton_alt, last);
    return static_cast<int>(n_unique);
  });
}

}  // namespace cpu