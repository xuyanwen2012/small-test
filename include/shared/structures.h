#pragma once

#include <cassert>
#include <glm/glm.hpp>
#include <stdexcept>

#include "defines.h"
#include "morton_func.h"

// I am using only pointers because this gives me a unified front end for both
// CPU/and GPU
struct RadixTree {
  const size_t capacity;

  // ------------------------
  // Essential Data
  // ------------------------
  int n_brt_nodes = UNINITIALIZED;

  uint8_t* u_prefix_n;
  bool* u_has_leaf_left;
  bool* u_has_leaf_right;
  int* u_left_child;
  int* u_parents;

  // ------------------------
  // Constructors
  // ------------------------

  RadixTree() = delete;

  explicit RadixTree(size_t n_to_allocate);

  RadixTree(const RadixTree&) = delete;
  RadixTree& operator=(const RadixTree&) = delete;
  RadixTree(RadixTree&&) = delete;
  RadixTree& operator=(RadixTree&&) = delete;

  ~RadixTree();

  // ------------------------
  // Getter/Setters
  // ------------------------

  void set_n_nodes(const size_t n_nodes) {
    assert(n_nodes < capacity);
    n_brt_nodes = static_cast<int>(n_nodes);
  }

  [[nodiscard]] int n_nodes() const {
    if (n_brt_nodes == UNINITIALIZED)
      throw std::runtime_error("BRT nodes unset!!!");
    return n_brt_nodes;
  }
};

struct Octree {
  const size_t capacity;

  // ------------------------
  // Essential Data
  // ------------------------

  int n_oct_nodes = UNINITIALIZED;

  // [Outputs]
  int (*u_children)[8];
  glm::vec4* u_corner;
  float* u_cell_size;
  int* u_child_node_mask;
  int* u_child_leaf_mask;

  // ------------------------
  // Constructors
  // ------------------------

  Octree() = delete;

  explicit Octree(size_t capacity);

  Octree(const Octree&) = delete;
  Octree& operator=(const Octree&) = delete;
  Octree(Octree&&) = delete;
  Octree& operator=(Octree&&) = delete;

  ~Octree();

  // ------------------------
  // Getter/Setters
  // ------------------------
  void set_n_nodes(const size_t n_nodes) {
    assert(n_nodes < capacity);
    n_oct_nodes = static_cast<int>(n_nodes);
  }

  [[nodiscard]] int n_nodes() const {
    if (n_oct_nodes == UNINITIALIZED)
      throw std::runtime_error("OCT nodes unset!!!");
    return n_oct_nodes;
  }
};

struct Pipe {
  // ------------------------
  // Essential Data (CPU/GPU shared)
  // ------------------------

  // mutable
  int n_unique = UNINITIALIZED;

  glm::vec4* u_points;
  morton_t* u_morton;
  morton_t* u_morton_alt;  // also used as the unique morton
  RadixTree brt;
  int* u_edge_counts;
  int* u_edge_offsets;
  Octree oct;

  // read-only
  int n_points;
  float min_coord;
  float range;
  int seed;

  // ------------------------
  // Temporary Storage (for GPU only)
  // only allocated when GPU is used
  // ------------------------

  static constexpr auto RADIX = 256;
  static constexpr auto RADIX_PASSES = 4;
  static constexpr auto BIN_PART_SIZE = 7680;
  static constexpr auto GLOBAL_HIST_THREADS = 128;
  static constexpr auto BINNING_THREADS = 512;

  size_t binning_blocks;

  struct {
    unsigned int* d_global_histogram;
    unsigned int* d_index;
    unsigned int* d_first_pass_histogram;
    unsigned int* d_second_pass_histogram;
    unsigned int* d_third_pass_histogram;
    unsigned int* d_fourth_pass_histogram;
    int* u_flag_heads;
  } im_storage;

  // ------------------------
  // Constructors
  // ------------------------

  Pipe() = delete;

  explicit Pipe(int n_points,
                float min_coord = 0.0f,
                float range = 1024.0f,
                int seed = 114514);

  Pipe(const Pipe&) = delete;
  Pipe& operator=(const Pipe&) = delete;
  Pipe(Pipe&&) = delete;
  Pipe& operator=(Pipe&&) = delete;

  ~Pipe();

  // ------------------------
  // Accessors (preffered over direct access)
  // ------------------------
  [[nodiscard]] int n_input() const { return n_points; }
  [[nodiscard]] int n_brt_nodes() const { return brt.n_nodes(); }

  [[nodiscard]] int n_unique_mortons() const {
    if (n_unique == UNINITIALIZED)
      throw std::runtime_error("Unique mortons unset!!!");
    return n_unique;
  }

  [[nodiscard]] int n_oct_nodes() const { return oct.n_nodes(); }

  void set_n_unique(const size_t n_unique) {
    assert(n_unique <= n_points);
    this->n_unique = static_cast<int>(n_unique);
  }

  // alias to make the code more understand able
  [[nodiscard]] const morton_t* getSortedKeys() const { return u_morton; }
  [[nodiscard]] morton_t* getUniqueKeys() { return u_morton_alt; }
  [[nodiscard]] const morton_t* getUniqueKeys() const { return u_morton_alt; }

  void clearSmem();
};
