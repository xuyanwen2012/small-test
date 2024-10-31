#pragma once

#include "memory_region.hpp"
#include <glm/glm.hpp>
#include <cassert>
#include <stdexcept>

static constexpr int UNINITIALIZED = -1;   
using morton_t = uint32_t;

// Base class for structures that need memory management
class ManagedStructure {
protected:
  const size_t capacity;
  std::unique_ptr<MemoryAllocator> allocator;

  explicit ManagedStructure(size_t capacity, Backend backend = Backend::CPU)
    : capacity(capacity), allocator(MemoryAllocator::create(backend)) {}
};

struct radix_tree : ManagedStructure {
  int n_brt_nodes = UNINITIALIZED;

  MemoryRegion prefix_n;
  MemoryRegion has_leaf_left;
  MemoryRegion has_leaf_right;
  MemoryRegion left_child;
  MemoryRegion parents;

  explicit radix_tree(size_t capacity, Backend backend = Backend::CPU);
  ~radix_tree();

  // Prevent copying and moving
  radix_tree(const radix_tree&) = delete;
  radix_tree& operator=(const radix_tree&) = delete;
  radix_tree(radix_tree&&) = delete;
  radix_tree& operator=(radix_tree&&) = delete;

  // Accessors
  uint8_t* get_prefix_n() { return prefix_n.as<uint8_t>(); }
  bool* get_has_leaf_left() { return has_leaf_left.as<bool>(); }
  bool* get_has_leaf_right() { return has_leaf_right.as<bool>(); }
  int* get_left_child() { return left_child.as<int>(); }
  int* get_parents() { return parents.as<int>(); }

  // Utility methods
  void set_n_nodes(const size_t n_nodes) {
    assert(n_nodes < capacity);
    n_brt_nodes = static_cast<int>(n_nodes);
  }

  [[nodiscard]] int n_nodes() const {
    if (n_brt_nodes == UNINITIALIZED)
      throw std::runtime_error("BRT nodes unset!");
    return n_brt_nodes;
  }
};

struct octree : ManagedStructure {
  int n_oct_nodes = UNINITIALIZED;

  MemoryRegion children;      // int[8] array
  MemoryRegion corner;        // glm::vec4 array
  MemoryRegion cell_size;     // float array
  MemoryRegion child_node_mask;
  MemoryRegion child_leaf_mask;

  explicit octree(size_t capacity, Backend backend = Backend::CPU);
  ~octree();

  // Prevent copying and moving
  octree(const octree&) = delete;
  octree& operator=(const octree&) = delete;
  octree(octree&&) = delete;
  octree& operator=(octree&&) = delete;

  // Accessors
  int (*get_children())[8] { return children.as<int[8]>(); }
  glm::vec4* get_corner() { return corner.as<glm::vec4>(); }
  float* get_cell_size() { return cell_size.as<float>(); }
  int* get_child_node_mask() { return child_node_mask.as<int>(); }
  int* get_child_leaf_mask() { return child_leaf_mask.as<int>(); }

  // Utility methods
  void set_n_nodes(const size_t n_nodes) {
    assert(n_nodes < capacity);
    n_oct_nodes = static_cast<int>(n_nodes);
  }

  [[nodiscard]] int n_nodes() const {
    if (n_oct_nodes == UNINITIALIZED)
      throw std::runtime_error("OCT nodes unset!");
    return n_oct_nodes;
  }
};

struct pipe : ManagedStructure {
  int n_unique = UNINITIALIZED;

  // Main storage
  MemoryRegion points;         // glm::vec4 array
  MemoryRegion morton;         // morton_t array
  MemoryRegion morton_alt;     // morton_t array (unique morton)
  MemoryRegion edge_counts;    // int array
  MemoryRegion edge_offsets;   // int array

  // Sub-structures
  radix_tree brt;
  octree oct;

  // Configuration
  const int n_points;
  const float min_coord;
  const float range;
  const int seed;

  // GPU-specific constants
  static constexpr auto RADIX = 256;
  static constexpr auto RADIX_PASSES = 4;
  static constexpr auto BIN_PART_SIZE = 7680;
  static constexpr auto GLOBAL_HIST_THREADS = 128;
  static constexpr auto BINNING_THREADS = 512;

  size_t binning_blocks;

  // Intermediate storage for GPU
  struct IntermediateStorage {
    MemoryRegion global_histogram;
    MemoryRegion index;
    MemoryRegion first_pass_histogram;
    MemoryRegion second_pass_histogram;
    MemoryRegion third_pass_histogram;
    MemoryRegion fourth_pass_histogram;
    MemoryRegion flag_heads;
  } im_storage;

  explicit pipe(int n_points,
                float min_coord = 0.0f,
                float range = 1024.0f,
                int seed = 114514,
                Backend backend = Backend::CPU);
  ~pipe();

  // Prevent copying and moving
  pipe(const pipe&) = delete;
  pipe& operator=(const pipe&) = delete;
  pipe(pipe&&) = delete;
  pipe& operator=(pipe&&) = delete;

  // Accessors
  glm::vec4* get_points() { return points.as<glm::vec4>(); }
  morton_t* get_morton() { return morton.as<morton_t>(); }
  morton_t* get_morton_alt() { return morton_alt.as<morton_t>(); }
  int* get_edge_counts() { return edge_counts.as<int>(); }
  int* get_edge_offsets() { return edge_offsets.as<int>(); }

  // Utility methods
  [[nodiscard]] int n_input() const { return n_points; }
  [[nodiscard]] int n_brt_nodes() const { return brt.n_nodes(); }
  [[nodiscard]] int n_oct_nodes() const { return oct.n_nodes(); }

  [[nodiscard]] int n_unique_mortons() const {
    if (n_unique == UNINITIALIZED)
      throw std::runtime_error("Unique mortons unset!");
    return n_unique;
  }

  void set_n_unique(const size_t n_unique) {
    assert(n_unique <= n_points);
    this->n_unique = static_cast<int>(n_unique);
  }

  // Convenience methods
  [[nodiscard]] const morton_t* getSortedKeys() const { return morton.as<morton_t>(); }
  [[nodiscard]] morton_t* getUniqueKeys() { return morton_alt.as<morton_t>(); }
  [[nodiscard]] const morton_t* getUniqueKeys() const { return morton_alt.as<morton_t>(); }
};
