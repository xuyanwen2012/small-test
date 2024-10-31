#pragma once

#include "core/thread_pool.hpp"
#include "shared/structures.h"
#include <memory>

namespace cpu {

/**
 * @brief Computes Morton codes for a set of 3D points in parallel
 * 
 * @param pool Thread pool to execute the work
 * @param num_threads Number of threads to use (must be > 0)
 * @param p Pipe containing input points and output morton codes
 * @return core::multi_future<void> Future representing completion of the work
 * @throws std::invalid_argument if num_threads <= 0 or p is null
 */
core::multi_future<void> dispatch_morton_code(
    core::thread_pool& pool,
    int num_threads,
    const std::shared_ptr<const Pipe>& p);

}  // namespace cpu
