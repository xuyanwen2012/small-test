#include "host_dispatcher.hpp"

#include <barrier>

#include "02_sort_impl.hpp"

namespace cpu {

core::multi_future<void> dispatch_MortonCode(
    core::thread_pool& pool,
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

void dispatch_RadixSort(core::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p) {
  std::barrier bar(num_threads);

  dispatch_binning_pass(
      pool, num_threads, bar, p->n_input(), p->u_morton, p->u_morton_alt, 0)
      .wait();
  dispatch_binning_pass(
      pool, num_threads, bar, p->n_input(), p->u_morton_alt, p->u_morton, 8)
      .wait();
  dispatch_binning_pass(
      pool, num_threads, bar, p->n_input(), p->u_morton, p->u_morton_alt, 16)
      .wait();
  dispatch_binning_pass(
      pool, num_threads, bar, p->n_input(), p->u_morton_alt, p->u_morton, 24)
      .wait();
}

}  // namespace cpu
