#pragma once

#include "core/thread_pool.hpp"
#include "shared/morton_func.h"

namespace cpu {

// ---------------------------------------------------------------------
// Morton encoding (1->1 relation)
// ---------------------------------------------------------------------

// raw pointer version

[[nodiscard]] inline core::multi_future<void> dispatch_morton_code(
    core::thread_pool &pool,
    const int desired_n_threads,
    const int n,
    const glm::vec4 *u_points,
    morton_t *u_morton,
    const float min_coord,
    const float range) {
  return pool.submit_blocks(
      0,
      n,
      [u_points, u_morton, min_coord, range](const int start, const int end) {
        for (int i = start; i < end; ++i) {
          u_morton[i] = shared::xyz_to_morton32(u_points[i], min_coord, range);
        }
      },
      desired_n_threads);
}

}  // namespace cpu