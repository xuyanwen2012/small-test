#include <spdlog/spdlog.h>

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
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
