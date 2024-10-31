#include "host_dispatcher.hpp"

namespace cpu {


core::multi_future<void> dispatch_morton_code(core::thread_pool& pool,
                                              int num_threads,
                                              const std::shared_ptr<const Pipe>& p) {
  return pool.submit_blocks(
      0,
      p->n_input(),
      [&](const int start, const int end) {
        for (int i = start; i < end; ++i) {
          p->u_morton[i] =
              shared::xyz_to_morton32(p->u_points[i], p->min_coord, p->range);
        }
      },
      num_threads);
}

}  // namespace cpu
