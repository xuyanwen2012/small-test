#include <benchmark/benchmark.h>

#include "../cpu/bm_config.hpp"
#include "spdlog/common.h"
#include "vulkan/engine.hpp"

// ---------------------------------------------------------------------------
// Morton Code
// ---------------------------------------------------------------------------

static void RunMortonCode(benchmark::State& state) {
  constexpr auto n_input = Config::DEFAULT_N;
  const auto n_blocks = state.range(0);

  Engine engine;

  auto u_points = engine.buffer(n_input * sizeof(glm::vec4));

  {
    std::random_device rd;
    std::mt19937 gen(Config::DEFAULT_SEED);
    std::uniform_real_distribution dis(
        Config::DEFAULT_MIN_COORD,
        Config::DEFAULT_MIN_COORD + Config::DEFAULT_RANGE);
    auto mapped = u_points->map<glm::vec4>();
    for (int i = 0; i < n_input; ++i) {
      mapped[i] = glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
    }
  }

  auto u_morton = engine.buffer(n_input * sizeof(uint32_t));
  u_morton->zeros();

  struct PushConstants {
    uint n;
    float min_coord;
    float range;
  };

  PushConstants pc = {static_cast<uint>(n_input),
                      Config::DEFAULT_MIN_COORD,
                      Config::DEFAULT_RANGE};

  auto algo = engine.algorithm("morton.spv",
                               {
                                   u_points,
                                   u_morton,
                               },
                               sizeof(pc));
  algo->set_push_constants(pc);
  auto seq = engine.sequence();

  for (auto _ : state) {
    seq->record_commands_with_blocks(algo.get(), n_blocks);
    seq->launch_kernel_async();
    seq->sync();
  }
}

BENCHMARK(RunMortonCode)
    ->DenseRange(1, 16, 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ---------------------------------------------------------------------------
// Merge Sort
// ---------------------------------------------------------------------------

static void RunMergeSort(benchmark::State& state) {
  constexpr auto n_input = Config::DEFAULT_N;
  const auto n_physical_blocks = state.range(0);

  Engine engine;

  auto u_input = engine.typed_buffer<uint32_t>(n_input);
  u_input->random(Config::DEFAULT_MIN_COORD, Config::DEFAULT_RANGE);

  auto u_output = engine.typed_buffer<uint32_t>(n_input);
  u_output->zeros();

  struct PushConstants {
    uint32_t n_logical_blocks;
    uint32_t n;
    uint32_t width;
    uint32_t num_pairs;
  };

  auto algorithm = engine.algorithm(
      "merge_sort.spv", {u_input, u_output}, sizeof(PushConstants));
  auto seq = engine.sequence();

  // ---------------------------------------------------------------------------
  constexpr auto threads_per_block = 256;

  for (auto _ : state) {
    // run merge sortS
    for (int width = 1; width < n_input; width *= 2) {
      int num_pairs = (n_input + 2 * width - 1) / (2 * width);
      int total_threads = num_pairs;
      int logical_blocks =
          (total_threads + threads_per_block - 1) / threads_per_block;

      PushConstants pc = {uint32_t(logical_blocks),
                          uint32_t(n_input),
                          uint32_t(width),
                          uint32_t(num_pairs)};
      algorithm->set_push_constants(pc);

      // should already be swapped by the previous iteration
      algorithm->update_descriptor_sets_with_buffers({u_input, u_output});

      seq->record_commands_with_blocks(algorithm.get(), n_physical_blocks);
      seq->launch_kernel_async();
      seq->sync();

      std::swap(u_input, u_output);
    }
  }
}

BENCHMARK(RunMergeSort)
    ->DenseRange(1, 8, 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(10);

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::off);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}