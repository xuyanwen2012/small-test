#pragma once

#include "application.hpp"
#include "init.hpp"
#include "morton.hpp"
#include "radix_sort_64.hpp"
#include "unique.hpp"
#include "radix_tree.hpp"
#include "edge_count.hpp"
#include "prefix_sum.hpp"
#include "downsweep.hpp"
#include "octree.hpp"
#include "octree.hpp"
#include <glm/vec4.hpp>
#include "app_params.hpp"
class Pipe : public ApplicationBase {
 public:
  Pipe(AppParams param) : ApplicationBase() {
    params_ = param;
  }
  void allocate();
  ~Pipe();
  void init(const int num_blocks, const int queue_idx);
  void morton(const int num_blocks, const int queue_idx);
  void radix_sort(const int num_blocks, const int queue_idx); 
  void radix_sort_alt(const int num_blocks, const int queue_idx);
  void unique(const int num_blocks, const int queue_idx);
  void radix_tree(const int num_blocks, const int queue_idx);
  void edge_count(const int num_blocks, const int queue_idx);
  void prefix_sum(const int num_blocks, const int queue_idx);
  void octree(const int num_blocks, const int queue_idx);

  protected:
  static constexpr auto educated_guess_nodes = 0.6f;
  AppParams params_;

  int n_pts;
  int n_unique_keys;
  int n_brt_nodes;  // unique_keys - 1
  int n_oct_nodes;  // computed late... we use 0.6 * n as a guess

  // Essential data memory
  glm::vec4* u_points;
  uint32_t* u_morton_keys;
  uint32_t* u_unique_morton_keys;
  int* u_edge_count;
  uint32_t* u_edge_offset;

  VkBuffer u_points_buffer;
  VkBuffer u_morton_keys_buffer;
  VkBuffer u_unique_morton_keys_buffer;
  VkBuffer u_edge_count_buffer;
  VkBuffer u_edge_offset_buffer;

  VkDeviceMemory u_points_memory;
  VkDeviceMemory u_morton_keys_memory;
  VkDeviceMemory u_unique_morton_keys_memory;
  VkDeviceMemory u_edge_count_memory;
  VkDeviceMemory u_edge_offset_memory;



  // Essential
  // should be of size 'n_unique_keys - 1', but we can allocate as 'n' for now
  struct {
    uint8_t* u_prefix_n;
    bool* u_has_leaf_left;  // you can't use vector of bools
    bool* u_has_leaf_right;
    int* u_left_child;
    int* u_parent;

    VkBuffer u_prefix_n_buffer;
    VkBuffer u_has_leaf_left_buffer;
    VkBuffer u_has_leaf_right_buffer;
    VkBuffer u_left_child_buffer;
    VkBuffer u_parent_buffer;

    VkDeviceMemory u_prefix_n_memory;
    VkDeviceMemory u_has_leaf_left_memory;
    VkDeviceMemory u_has_leaf_right_memory;
    VkDeviceMemory u_left_child_memory;
    VkDeviceMemory u_parent_memory;
  } brt;

  // Essential
  // should be of size 'n_oct_nodes', we use an educated guess for now
  struct {
    OctNode* u_nodes;


    VkBuffer u_nodes_buffer;

    VkDeviceMemory u_nodes_memory;
  } oct;

  // Temp
  struct {
    uint32_t* u_sort_alt;              // n
    uint32_t* u_global_histogram;      // 256 * 4
    uint32_t* u_index;                 // 4
    glm::uvec4* u_pass_histogram;  // 256 * xxx
    uint32_t* u_pass_histogram_64;

    VkBuffer u_sort_alt_buffer;
    VkBuffer u_global_histogram_buffer;
    VkBuffer u_index_buffer;
    VkBuffer u_pass_histogram_buffer;
    VkBuffer u_pass_histogram_64_buffer;

    VkDeviceMemory u_sort_alt_memory;
    VkDeviceMemory u_global_histogram_memory;
    VkDeviceMemory u_index_memory;
    VkDeviceMemory u_pass_histogram_memory;
    VkDeviceMemory u_pass_histogram_64_memory;
  } sort_tmp;

  struct {
    uint32_t* contributions;  // n
    volatile uint32_t* index;
    volatile uint32_t* reductions;

    VkBuffer contributions_buffer;
    VkBuffer index_buffer;
    VkBuffer reductions_buffer;

    VkDeviceMemory contributions_memory;
    VkDeviceMemory index_memory;
    VkDeviceMemory reductions_memory;
  } unique_tmp;
  
  struct {
    uint32_t* index;
    uint32_t* reductions;

    VkBuffer index_buffer;
    VkBuffer reductions_buffer;

    VkDeviceMemory index_memory;
    VkDeviceMemory reductions_memory;
  } prefix_sum_tmp;
  
};


void Pipe::allocate() {
    void *mapped;
    std::cout << "Starting buffer allocation..." << std::endl;
    
    // Add try-catch or error checking for each allocation
    try {
        create_shared_empty_storage_buffer(params_.n * sizeof(glm::vec4), 
            &u_points_buffer, &u_points_memory, &mapped);
        u_points = static_cast<glm::vec4*>(mapped);
        std::cout << "Points buffer allocated successfully" << std::endl;
        // ... rest of allocations
    } catch (const std::exception& e) {
        std::cerr << "Error during allocation: " << e.what() << std::endl;
    }
}

Pipe::~Pipe() {
  // --- Essentials ---
  vkUnmapMemory(singleton.device, u_points_memory);
  vkDestroyBuffer(singleton.device, u_points_buffer, nullptr);
  vkFreeMemory(singleton.device, u_points_memory, nullptr);

  vkUnmapMemory(singleton.device, u_morton_keys_memory);
  vkDestroyBuffer(singleton.device, u_morton_keys_buffer, nullptr);
  vkFreeMemory(singleton.device, u_morton_keys_memory, nullptr);

  vkUnmapMemory(singleton.device, u_unique_morton_keys_memory);
  vkDestroyBuffer(singleton.device, u_unique_morton_keys_buffer, nullptr);
  vkFreeMemory(singleton.device, u_unique_morton_keys_memory, nullptr);

  vkUnmapMemory(singleton.device, u_edge_count_memory);
  vkDestroyBuffer(singleton.device, u_edge_count_buffer, nullptr);
  vkFreeMemory(singleton.device, u_edge_count_memory, nullptr);

  vkUnmapMemory(singleton.device, u_edge_offset_memory);
  vkDestroyBuffer(singleton.device, u_edge_offset_buffer, nullptr);
  vkFreeMemory(singleton.device, u_edge_offset_memory, nullptr);
  
  // brt
  vkUnmapMemory(singleton.device, brt.u_prefix_n_memory);
  vkDestroyBuffer(singleton.device, brt.u_prefix_n_buffer, nullptr);
  vkFreeMemory(singleton.device, brt.u_prefix_n_memory, nullptr);

  vkUnmapMemory(singleton.device, brt.u_has_leaf_left_memory);
  vkDestroyBuffer(singleton.device, brt.u_has_leaf_left_buffer, nullptr);
  vkFreeMemory(singleton.device, brt.u_has_leaf_left_memory, nullptr);

  vkUnmapMemory(singleton.device, brt.u_has_leaf_right_memory);
  vkDestroyBuffer(singleton.device, brt.u_has_leaf_right_buffer, nullptr);
  vkFreeMemory(singleton.device, brt.u_has_leaf_right_memory, nullptr);

  vkUnmapMemory(singleton.device, brt.u_left_child_memory);
  vkDestroyBuffer(singleton.device, brt.u_left_child_buffer, nullptr);
  vkFreeMemory(singleton.device, brt.u_left_child_memory, nullptr);

  vkUnmapMemory(singleton.device, brt.u_parent_memory);
  vkDestroyBuffer(singleton.device, brt.u_parent_buffer, nullptr);
  vkFreeMemory(singleton.device, brt.u_parent_memory, nullptr);

  // oct
  vkUnmapMemory(singleton.device, oct.u_nodes_memory);
  vkDestroyBuffer(singleton.device, oct.u_nodes_buffer, nullptr);
  vkFreeMemory(singleton.device, oct.u_nodes_memory, nullptr);

  // -------------------------

  // Temporary storages for Sort
  vkUnmapMemory(singleton.device, sort_tmp.u_sort_alt_memory);
  vkDestroyBuffer(singleton.device, sort_tmp.u_sort_alt_buffer, nullptr);
  vkFreeMemory(singleton.device, sort_tmp.u_sort_alt_memory, nullptr);

  vkUnmapMemory(singleton.device, sort_tmp.u_global_histogram_memory);
  vkDestroyBuffer(singleton.device, sort_tmp.u_global_histogram_buffer, nullptr);
  vkFreeMemory(singleton.device, sort_tmp.u_global_histogram_memory, nullptr);

  vkUnmapMemory(singleton.device, sort_tmp.u_index_memory);
  vkDestroyBuffer(singleton.device, sort_tmp.u_index_buffer, nullptr);
  vkFreeMemory(singleton.device, sort_tmp.u_index_memory, nullptr);

  vkUnmapMemory(singleton.device, sort_tmp.u_pass_histogram_memory);
  vkDestroyBuffer(singleton.device, sort_tmp.u_pass_histogram_buffer, nullptr);
  vkFreeMemory(singleton.device, sort_tmp.u_pass_histogram_memory, nullptr);

  vkUnmapMemory(singleton.device, sort_tmp.u_pass_histogram_64_memory);
  vkDestroyBuffer(singleton.device, sort_tmp.u_pass_histogram_64_buffer, nullptr);
  vkFreeMemory(singleton.device, sort_tmp.u_pass_histogram_64_memory, nullptr);


  // Temporary storages for Unique
  vkUnmapMemory(singleton.device, unique_tmp.contributions_memory);
  vkDestroyBuffer(singleton.device, unique_tmp.contributions_buffer, nullptr);
  vkFreeMemory(singleton.device, unique_tmp.contributions_memory, nullptr);

  // Temporary storages for PrefixSum

  vkUnmapMemory(singleton.device, prefix_sum_tmp.index_memory);
  vkDestroyBuffer(singleton.device, prefix_sum_tmp.index_buffer, nullptr);
  vkFreeMemory(singleton.device, prefix_sum_tmp.index_memory, nullptr);

  vkUnmapMemory(singleton.device, prefix_sum_tmp.reductions_memory);
  vkDestroyBuffer(singleton.device, prefix_sum_tmp.reductions_buffer, nullptr);
  vkFreeMemory(singleton.device, prefix_sum_tmp.reductions_memory, nullptr);
  
}

void Pipe::init(const int num_blocks, const int queue_idx){
  std::cout << "start init"<<std::endl;
  Init init_stage = Init();
  init_stage.run(num_blocks, queue_idx, u_points, u_points_buffer, params_.n, params_.min_coord, params_.getRange(), params_.seed);
  /*
  for (int i = 0; i < 1024; ++i){
    std::cout << u_points[i].x << " " << u_points[i].y << " " << u_points[i].z << " " << u_points[i].w << std::endl;
  }
  */
}

void Pipe::morton(const int num_blocks, const int queue_idx){
  std::cout << "start morton"<<std::endl;
  Morton morton_stage = Morton();
  morton_stage.run(num_blocks, queue_idx, u_points, u_morton_keys, u_points_buffer, u_morton_keys_buffer,  params_.n, params_.min_coord, params_.getRange());
  /*
  for (int i = 0; i < 1024; i++){
    printf("morton_keys[%d]: %d\n", i, u_morton_keys[i]);
  }
  */
 /*
 for(int i = 0; i < params_.n; ++i){
    u_morton_keys[i] = params_.n - i - 1;
 }
 */
 
}


void Pipe::radix_sort_alt(const int num_blocks, const int queue_idx){
  std::cout << "start radix sort alt"<<std::endl;
    for (int i = 0; i < 1024; i++){
    printf("unsorted_key[%d]: %d\n", i, u_morton_keys[i]);
  }
  auto radixsort_stage = RadixSort64();
  radixsort_stage.run(num_blocks,
  queue_idx,
  u_morton_keys,
  sort_tmp.u_sort_alt,
  sort_tmp.u_global_histogram,
  sort_tmp.u_index,
  sort_tmp.u_pass_histogram_64,
  u_morton_keys_buffer,
  sort_tmp.u_sort_alt_buffer,
  sort_tmp.u_global_histogram_buffer,
  sort_tmp.u_index_buffer,
  sort_tmp.u_pass_histogram_64_buffer,
  params_.n);
  // std::sort(u_morton_keys, u_morton_keys + params_.n);
  
  for (int i = 0; i < 1024; ++i){
    printf("global_histogram[%d]: %d\n", i, sort_tmp.u_global_histogram[i]);
  }
  for (int i = 0; i < 1024; i++){
    printf("pass_histogram[%d]: %d\n", i, sort_tmp.u_pass_histogram_64[i]);
  }
  
  for (int i = 0; i < 256; i++){
    printf("sorted_key[%d]: %d\n", i, u_morton_keys[i]);
  }

  
   for (int i = params_.n-200; i < params_.n; i++){
     printf("sorted_key[%d]: %d\n", i, u_morton_keys[i]);
   }

}

void Pipe::unique(const int num_blocks, const int queue_idx){
  std::cout << "start unique"<<std::endl;
  auto unique_stage = Unique();
  unique_stage.run(num_blocks,
  queue_idx,
  u_morton_keys,
  u_unique_morton_keys,
  unique_tmp.contributions,
  unique_tmp.reductions,
  unique_tmp.index,
  u_morton_keys_buffer,
  u_unique_morton_keys_buffer,
  unique_tmp.contributions_buffer,
  unique_tmp.reductions_buffer,
  unique_tmp.index_buffer,
  params_.n);

  for (int i = 0; i < 200; ++i){
    unique_tmp.contributions[i] = i;
  }

  for (int i = 0; i < 200; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);
  

  for(int i = 3072; i < 3272; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);

  for (int i = 3072*2; i < 3072*3; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);
  
  for (int i = 3072*3; i < 3072*4; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);
  
  for (int i = 3072*4; i < 3072*5; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);

  for (int i = params_.n-3072; i < params_.n; ++i)
    printf("contributions[%d]: %d\n", i, unique_tmp.contributions[i]);
  

  for(int i = params_.n-200; i < params_.n; i++)
    printf("unique_morton_keys[%d]: %d\n", i, u_unique_morton_keys[i]);
  
  n_unique_keys = unique_tmp.contributions[params_.n-1];
  printf("n_unique_keys: %d\n", n_unique_keys);

}

void Pipe::radix_tree(const int num_blocks, const int queue_idx){
  std::cout << "start radix tree"<<std::endl;
  auto radix_tree_stage = RadixTree();
  radix_tree_stage.run(num_blocks,
  queue_idx,
  u_unique_morton_keys,
  brt.u_prefix_n,
  brt.u_has_leaf_left,
  brt.u_has_leaf_right,
  brt.u_left_child,
  brt.u_parent,
  u_unique_morton_keys_buffer,
  brt.u_prefix_n_buffer,
  brt.u_has_leaf_left_buffer,
  brt.u_has_leaf_right_buffer,
  brt.u_left_child_buffer, 
  brt.u_parent_buffer,
  n_unique_keys);
}



void Pipe::edge_count(const int num_blocks, const int queue_idx){
  std::cout <<"start edge count"<<std::endl;
  auto edge_count_stage = EdgeCount();
  edge_count_stage.run(num_blocks, queue_idx, brt.u_prefix_n , brt.u_parent, u_edge_count, brt.u_prefix_n_buffer, brt.u_parent_buffer, u_edge_count_buffer, n_unique_keys);
  /*
  for (int i = 0; i < 1024; i++){
    printf("edge_count[%d]: %d\n", i, u_edge_count[i]);
  }
  */
}

void Pipe::prefix_sum(const int num_blocks, const int queue_idx){
  std::cout << "start prefix sum"<<std::endl;
  memcpy(u_edge_offset, u_edge_count,  sizeof(uint32_t) * params_.n);
  auto prefix_sum_stage = DownSweepPrefixSum();
  //auto prefix_sum_stage = PrefixSum();
  prefix_sum_stage.run(num_blocks,
  queue_idx,
  u_edge_offset,
  prefix_sum_tmp.reductions,
  prefix_sum_tmp.index,
  u_edge_offset_buffer,
  prefix_sum_tmp.reductions_buffer,
  prefix_sum_tmp.index_buffer,

  params_.n);

  printf("last element: %d\n", u_edge_offset[params_.n-1]);
  
}

void Pipe::octree(const int num_blocks, const int queue_idx){
   std::cout << "start octree"<<std::endl;
  std::cout<<"start octree"<<std::endl;
  n_brt_nodes = n_unique_keys - 1;
  auto build_octree_stage = Octree();
  build_octree_stage.run(num_blocks,
  queue_idx,
  oct.u_nodes,
  u_edge_offset,
  u_edge_count,
  u_unique_morton_keys,
  brt.u_prefix_n,
  brt.u_has_leaf_left,
  brt.u_has_leaf_right,
  brt.u_parent,
  brt.u_left_child,
  params_.min_coord,
  params_.getRange(),
  n_brt_nodes,
  oct.u_nodes_buffer,
  u_edge_offset_buffer,
  u_edge_count_buffer,
  u_unique_morton_keys_buffer,
  brt.u_prefix_n_buffer,
  brt.u_has_leaf_left_buffer,
  brt.u_has_leaf_right_buffer,
  brt.u_parent_buffer,
  brt.u_left_child_buffer);
  std::cout << "done octree"<<std::endl;

}
