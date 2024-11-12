#include <spdlog/spdlog.h>

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

#include "glm_helper.hpp"
#include "shared/morton_func.h"
#include "third-party/CLI11.hpp"
#include "vulkan/engine.hpp"

// struct Pipe {
//   // ------------------------
//   // Essential Data (CPU/GPU shared)
//   // ------------------------

//   // mutable
//   int n_unique = UNINITIALIZED;

//   glm::vec4* u_points;
//   morton_t* u_morton;
//   morton_t* u_morton_alt;  // also used as the unique morton
//   // RadixTree brt;
//   int* u_edge_counts;
//   int* u_edge_offsets;
//   // Octree oct;

//   // read-only
//   int n_points;
//   float min_coord;
//   float range;
//   int seed;
// };

struct Pipe {
  // ------------------------
  // Essential Data (CPU/GPU shared)
  // ------------------------

  // mutable
  int n_unique = UNINITIALIZED;

  // glm::vec4* u_points;
  // morton_t* u_morton;
  // morton_t* u_morton_alt;  // also used as the unique morton
  // // RadixTree brt;
  // int* u_edge_counts;
  // int* u_edge_offsets;
  // // Octree oct;

  // Buffer<glm::vec4> u_points;
  // Buffer<morton_t> u_morton;
  // Buffer<morton_t> u_morton_alt;  // also used as the unique morton
  // Buffer<int> u_edge_counts;
  // Buffer<int> u_edge_offsets;

  // read-only
  int n_points;
  float min_coord;
  float range;
  int seed;
};

int main(int argc, char** argv) {
  CLI::App app{"VMA Debug"};

  // parameters
  bool debug = false;
  int num_blocks = 1;

  app.add_flag("--debug", debug, "Enable debug mode");
  app.add_option("-b,--blocks", num_blocks, "Number of blocks")->default_val(1);
  app.allow_extras();

  CLI11_PARSE(app, argc, argv);

  spdlog::set_level(debug ? spdlog::level::debug : spdlog::level::info);

  Engine engine;

  constexpr int n = 1024;
  constexpr int min_val = 0;
  constexpr int range = 1;
  constexpr int seed = 114514;

  // print all parameters and configs
  spdlog::info("n: {}", n);
  spdlog::info("min_val: {}", min_val);
  spdlog::info("range: {}", range);
  spdlog::info("seed: {}", seed);
  spdlog::info("num_blocks: {}", num_blocks);

  {
    auto init_data = engine.buffer(n * sizeof(glm::vec4));
    init_data->zeros();

    // Step 1: Initialize the data
    struct {
      int n;
      int min_val;
      int range;
      int seed;
    } init_push_constants = {n, min_val, range, seed};

    auto algo = engine.algorithm(
        "init.spv",
        {init_data},
        512,
        reinterpret_cast<const std::byte*>(&init_push_constants),
        sizeof(init_push_constants));

    auto seq = engine.sequence();
    seq->record_commands_with_blocks(algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();

    // print 10 results
    auto init_span = init_data->span<glm::vec4>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("{}: {}", i, init_span[i]);
    }

    // Step 2: Compute the Morton code
    auto morton_buf = engine.buffer(n * sizeof(uint32_t));
    morton_buf->zeros();

    struct {
      int n;
      float min_coord;
      float range;
    } morton_push_constants = {n, min_val, range};

    auto morton_algo = engine.algorithm(
        "morton.spv",
        {init_data, morton_buf},
        768,
        reinterpret_cast<const std::byte*>(&morton_push_constants),
        sizeof(morton_push_constants));

    seq->record_commands_with_blocks(morton_algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();

    // print 10 results
    auto morton_span = morton_buf->span<uint32_t>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("Morton code {}: {}", i, morton_span[i]);
    }

    // Step 3: Sort the Morton code (currently CPU)
    std::ranges::sort(morton_span);

    // print 10 results
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("Sorted Morton code {}: {}", i, morton_span[i]);
    }

    // // Step 4: Compute the unique Morton code
    // auto contributes_buf = engine.buffer(n * sizeof(uint32_t));
    // contributes_buf->zeros();

    // struct {
    //   int n;
    // } find_dups_push_constants = {n};

    // auto find_dups_algo = engine.algorithm(
    //     "find_dups.spv",
    //     {contributes_buf, morton_buf},
    //     256,
    //     reinterpret_cast<const std::byte*>(&find_dups_push_constants),
    //     sizeof(find_dups_push_constants));

    // seq->record_commands_with_blocks(find_dups_algo.get(), num_blocks);
    // seq->launch_kernel_async();
    // seq->sync();

    // auto out_keys_buf = engine.buffer(n * sizeof(uint32_t));
    // out_keys_buf->zeros();

    // auto out_idx_buf = engine.buffer(n * sizeof(uint32_t));
    // out_idx_buf->zeros();

    // // Step 4.5: Move the duplicates
    // struct {
    //   int n;
    // } move_dups_push_constants = {n};

    // auto move_dups_algo = engine.algorithm(
    //     "move_dups.spv",
    //     {contributes_buf, morton_buf, out_keys_buf, out_idx_buf},
    //     256,
    //     reinterpret_cast<const std::byte*>(&move_dups_push_constants),
    //     sizeof(move_dups_push_constants));

    // seq->record_commands_with_blocks(move_dups_algo.get(), num_blocks);
    // seq->launch_kernel_async();
    // seq->sync();

    // // print 10 results
    // auto out_keys_span = out_keys_buf->span<uint32_t>();
    // for (auto i = 0; i < 10; ++i) {
    //   spdlog::info("Out keys {}: {}", i, out_keys_span[i]);
    // }

    auto unique_morton = engine.buffer(n * sizeof(uint32_t));
    unique_morton->zeros();

    std::unique_copy(morton_span.begin(),
                     morton_span.end(),
                     unique_morton->span<uint32_t>().begin());
    const auto n_unique = std::distance(unique_morton->span<uint32_t>().begin(),
                                        unique_morton->span<uint32_t>().end());

    // print 10 results
    auto unique_morton_span = unique_morton->span<uint32_t>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("Unique Morton code {}: {}", i, unique_morton_span[i]);
    }

    spdlog::info("n_unique: {}", n_unique);

    // Step 5: Radix tree build

    //     layout(push_constant) uniform N { int n; };

    // layout(set = 0, binding = 0) buffer Codes { uint codes[]; };

    // layout(set = 0, binding = 1) buffer PrefixN { uint8_t prefix_n[]; };

    // layout(set = 0, binding = 2) buffer HasLeafLeft { bool has_leaf_left[];
    // };

    // layout(set = 0, binding = 3) buffer HasLeafRight { bool has_leaf_right[];
    // };

    // layout(set = 0, binding = 4) buffer LeftChild { int left_child[]; };

    // layout(set = 0, binding = 5) buffer Parent { int parent[]; };

    // layout(local_size_x = 256) in;

    auto prefix_n_buf = engine.buffer(n_unique * sizeof(uint8_t));
    prefix_n_buf->zeros();

    auto has_leaf_left_buf = engine.buffer(n_unique * sizeof(bool));
    has_leaf_left_buf->zeros();

    auto has_leaf_right_buf = engine.buffer(n_unique * sizeof(bool));
    has_leaf_right_buf->zeros();

    auto left_child_buf = engine.buffer(n_unique * sizeof(int));
    left_child_buf->zeros();

    auto parent_buf = engine.buffer(n_unique * sizeof(int));
    parent_buf->zeros();

    struct {
      int n;
    } build_radix_tree_push_constants = {static_cast<int>(n_unique)};

    auto build_radix_tree_algo = engine.algorithm(
        "build_radix_tree.spv",
        {unique_morton,
         prefix_n_buf,
         has_leaf_left_buf,
         has_leaf_right_buf,
         left_child_buf,
         parent_buf},
        256,
        reinterpret_cast<const std::byte*>(&build_radix_tree_push_constants),
        sizeof(build_radix_tree_push_constants));

    seq->record_commands_with_blocks(build_radix_tree_algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();

    // print 10 results
    auto prefix_n_span = prefix_n_buf->span<uint8_t>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("\tPrefix N {}: {}", i, prefix_n_span[i]);
      spdlog::info(
          "\tHas leaf left {}: {}", i, has_leaf_left_buf->span<bool>()[i]);
      spdlog::info(
          "\tHas leaf right {}: {}", i, has_leaf_right_buf->span<bool>()[i]);
      spdlog::info("\tLeft child {}: {}", i, left_child_buf->span<int>()[i]);
      spdlog::info("\tParent {}: {}", i, parent_buf->span<int>()[i]);
    }

    const auto n_brt_nodes = n_unique - 1;

    // Step 6: edge counts
    //     layout(set = 0, binding = 0) buffer PrefixN { uint8_t prefix_n[]; };
    // layout(set = 0, binding = 1) buffer Parent { int parent[]; };
    // layout(set = 0, binding = 2) buffer EdgeCount { int edge_count[]; };

    // layout(push_constant) uniform Constant { int n_brt_nodes; };

    // layout(local_size_x = 512) in;

    auto edge_count_buf = engine.buffer(n_brt_nodes * sizeof(int));
    edge_count_buf->zeros();

    struct {
      int n_brt_nodes;
    } edge_count_push_constants = {static_cast<int>(n_brt_nodes)};

    auto edge_count_algo = engine.algorithm(
        "edge_count.spv",
        {prefix_n_buf, parent_buf, edge_count_buf},
        512,
        reinterpret_cast<const std::byte*>(&edge_count_push_constants),
        sizeof(edge_count_push_constants));

    seq->record_commands_with_blocks(edge_count_algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();

    // print 10 results
    auto edge_count_span = edge_count_buf->span<int>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("\tEdge count {}: {}", i, edge_count_span[i]);
    }

    // Step 7: edge offsets (prefix sum), let use cpu for now

    auto edge_offsets_buf = engine.buffer(n_brt_nodes * sizeof(int));
    edge_offsets_buf->zeros();

    std::partial_sum(edge_count_span.begin(),
                     edge_count_span.end(),
                     edge_offsets_buf->span<int>().begin());

    // print 10 results
    auto edge_offsets_span = edge_offsets_buf->span<int>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("\tEdge offsets {}: {}", i, edge_offsets_span[i]);
    }
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
