#include "host_dispatcher.hpp"

#include <barrier>
#include <numeric>

#include "02_sort_impl.hpp"
#include "host/brt_func.hpp"
#include "shared/edge_func.h"
#include "shared/oct_func.h"

namespace cpu {

void dispatch_MortonCode(core::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p) {
  pool.submit_blocks(
          0,
          p->n_input(),
          [&](const int start, const int end) {
            for (int i = start; i < end; ++i) {
              p->u_morton[i] = shared::xyz_to_morton32(
                  p->u_points[i], p->min_coord, p->range);
            }
          },
          num_threads)
      .wait();
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

void dispatch_RemoveDuplicates(core::thread_pool& pool,
                               int num_threads,
                               const std::shared_ptr<Pipe>& p) {
  const auto last = std::unique_copy(
      p->u_morton, p->u_morton + p->n_input(), p->u_morton_alt);
  const auto n_unique = std::distance(p->u_morton_alt, last);
  p->set_n_unique(n_unique);
  p->brt.set_n_nodes(n_unique - 1);
}

void dispatch_BuildRadixTree(core::thread_pool& pool,
                             int num_threads,
                             const std::shared_ptr<const Pipe>& p) {
  return pool
      .submit_blocks(
          0,
          p->brt.n_nodes(),
          [p](const int start, const int end) {
            for (auto i = start; i < end; ++i) {
              cpu::process_radix_tree_i(
                  i, p->brt.n_nodes(), p->u_morton, &p->brt);
            }
          },
          num_threads)
      .wait();
}

void dispatch_EdgeCount(core::thread_pool& pool,
                        int num_threads,
                        const std::shared_ptr<const Pipe>& p) {
  pool.submit_blocks(
          0,
          p->brt.n_nodes(),
          [p](const int start, const int end) {
            for (auto i = start; i < end; ++i) {
              shared::process_edge_count_i(
                  i, p->brt.u_prefix_n, p->brt.u_parents, p->u_edge_counts);
            }
          },
          num_threads)
      .wait();
}

void dispatch_EdgeOffset(core::thread_pool& pool,
                         int num_threads,
                         const std::shared_ptr<const Pipe>& p) {
  pool.submit_task([p]() {
        std::partial_sum(p->u_edge_counts,
                         p->u_edge_counts + p->n_brt_nodes(),
                         p->u_edge_offsets);
        return p->u_edge_offsets[p->brt.n_nodes() - 1];
      })
      .wait();
}

void dispatch_BuildOctree(core::thread_pool& pool,
                          int num_threads,
                          const std::shared_ptr<const Pipe>& p) {
  pool.submit_blocks(
          1,
          p->brt.n_nodes(),
          [p](const int start, const int end) {
            for (auto i = start; i < end; ++i) {
              shared::process_oct_node(i,
                                       p->oct.u_children,
                                       p->oct.u_corner,
                                       p->oct.u_cell_size,
                                       p->oct.u_child_node_mask,
                                       p->u_edge_offsets,
                                       p->u_edge_counts,
                                       p->u_morton,
                                       p->brt.u_prefix_n,
                                       p->brt.u_parents,
                                       p->min_coord,
                                       p->range);
            }
          },
          num_threads)
      .wait();
  pool.submit_blocks(
          0,
          p->brt.n_nodes(),
          [p](const int start, const int end) {
            for (auto i = start; i < end; ++i) {
              shared::process_link_leaf(i,
                                        p->oct.u_children,
                                        p->oct.u_child_leaf_mask,
                                        p->u_edge_offsets,
                                        p->u_edge_counts,
                                        p->u_morton,
                                        p->brt.u_has_leaf_left,
                                        p->brt.u_has_leaf_right,
                                        p->brt.u_prefix_n,
                                        p->brt.u_parents,
                                        p->brt.u_left_child);
            }
          },
          num_threads)
      .wait();
}

}  // namespace cpu
