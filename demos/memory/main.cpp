#include <cstddef>
#include <iostream>

#include "memory_region.hpp"

#include <glm/glm.hpp>

using morton_t = unsigned int;

struct RadixTree {
  const size_t capacity;
  int n_brt_nodes;

  // Essential Data
  MemoryRegion<uint8_t> u_prefix_n;
  MemoryRegion<bool> u_has_leaf_left;
  MemoryRegion<bool> u_has_leaf_right;
  MemoryRegion<int> u_left_child;
  MemoryRegion<int> u_parents;

  RadixTree() = delete;
  explicit RadixTree(size_t capacity)
      : capacity(capacity),
        u_prefix_n(capacity),
        u_has_leaf_left(capacity),
        u_has_leaf_right(capacity),
        u_left_child(capacity),
        u_parents(capacity) {}

  // getters
  size_t get_capacity() const { return capacity; }
  int get_n_brt_nodes() const { return n_brt_nodes; }

  // setters
  void set_n_brt_nodes(int n_brt_nodes) { this->n_brt_nodes = n_brt_nodes; }
};

struct Pipe {


  MemoryRegion<glm::vec4> u_points;
  MemoryRegion<morton_t> u_morton;
  MemoryRegion<morton_t> u_morton_alt;
  RadixTree brt;
  MemoryRegion<int> u_edge_counts;
  MemoryRegion<int> u_edge_offsets;
  // MemoryRegion<octree> oct;
};

int main() {
  auto tree = std::make_unique<RadixTree>(1024);

  std::cout << "Capacity: " << tree->get_capacity() << '\n';
  std::cout << "Number of BRT nodes: " << tree->get_n_brt_nodes() << '\n';

  auto pipe = std::make_unique<Pipe>();
  

  return 0;
}
