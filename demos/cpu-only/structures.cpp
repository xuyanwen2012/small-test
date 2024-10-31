#include "structures.hpp"

radix_tree::radix_tree(size_t capacity, Backend backend)
    : ManagedStructure(capacity, backend) {
  prefix_n = allocator->allocate(capacity * sizeof(uint8_t));
  has_leaf_left = allocator->allocate(capacity * sizeof(bool));
  has_leaf_right = allocator->allocate(capacity * sizeof(bool));
  left_child = allocator->allocate(capacity * sizeof(int));
  parents = allocator->allocate(capacity * sizeof(int));
}

radix_tree::~radix_tree() {
  if (allocator) {
    allocator->deallocate(prefix_n);
    allocator->deallocate(has_leaf_left);
    allocator->deallocate(has_leaf_right);
    allocator->deallocate(left_child);
    allocator->deallocate(parents);
  }
}

octree::octree(size_t capacity, Backend backend)
    : ManagedStructure(capacity, backend) {
  children = allocator->allocate(capacity * sizeof(int[8]));
  corner = allocator->allocate(capacity * sizeof(glm::vec4));
  cell_size = allocator->allocate(capacity * sizeof(float));
  child_node_mask = allocator->allocate(capacity * sizeof(int));
  child_leaf_mask = allocator->allocate(capacity * sizeof(int));
}

octree::~octree() {
  if (allocator) {
    allocator->deallocate(children);
    allocator->deallocate(corner);
    allocator->deallocate(cell_size);
    allocator->deallocate(child_node_mask);
    allocator->deallocate(child_leaf_mask);
  }
}

pipe::pipe(
    int n_points, float min_coord, float range, int seed, Backend backend)
    : ManagedStructure(n_points, backend),
      n_points(n_points),
      min_coord(min_coord),
      range(range),
      seed(seed),
      brt(n_points, backend),
      oct(n_points, backend) {
  points = allocator->allocate(n_points * sizeof(glm::vec4));
  morton = allocator->allocate(n_points * sizeof(morton_t));
  morton_alt = allocator->allocate(n_points * sizeof(morton_t));
  edge_counts = allocator->allocate(n_points * sizeof(int));
  edge_offsets = allocator->allocate(n_points * sizeof(int));

  binning_blocks = (n_points + BIN_PART_SIZE - 1) / BIN_PART_SIZE;

#ifdef ENABLE_CUDA
  if (backend == Backend::CUDA) {
    // Allocate GPU intermediate storage
    im_storage.global_histogram =
        allocator->allocate(RADIX * sizeof(unsigned int));
    im_storage.index =
        allocator->allocate(binning_blocks * RADIX * sizeof(unsigned int));
    im_storage.first_pass_histogram =
        allocator->allocate(binning_blocks * RADIX * sizeof(unsigned int));
    im_storage.second_pass_histogram =
        allocator->allocate(binning_blocks * RADIX * sizeof(unsigned int));
    im_storage.third_pass_histogram =
        allocator->allocate(binning_blocks * RADIX * sizeof(unsigned int));
    im_storage.fourth_pass_histogram =
        allocator->allocate(binning_blocks * RADIX * sizeof(unsigned int));
    im_storage.flag_heads = allocator->allocate(n_points * sizeof(int));
  }
#endif
}

pipe::~pipe() {
  if (allocator) {
    allocator->deallocate(points);
    allocator->deallocate(morton);
    allocator->deallocate(morton_alt);
    allocator->deallocate(edge_counts);
    allocator->deallocate(edge_offsets);

#ifdef ENABLE_CUDA
    if (im_storage.global_histogram.ptr) {
      allocator->deallocate(im_storage.global_histogram);
      allocator->deallocate(im_storage.index);
      allocator->deallocate(im_storage.first_pass_histogram);
      allocator->deallocate(im_storage.second_pass_histogram);
      allocator->deallocate(im_storage.third_pass_histogram);
      allocator->deallocate(im_storage.fourth_pass_histogram);
      allocator->deallocate(im_storage.flag_heads);
    }
#endif
  }
}