#include <cstddef>

#include "memory_region.hpp"

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
      : capacity(capacity), u_prefix_n(capacity), u_has_leaf_left(capacity),
        u_has_leaf_right(capacity), u_left_child(capacity), u_parents(capacity) {
    
  }


};

int main() {

    auto tree = std::make_unique<RadixTree>(1024);

    


  return 0;
}


