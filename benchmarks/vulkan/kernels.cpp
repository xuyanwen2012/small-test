#include <benchmark/benchmark.h>

#include "../cpu/bm_config.hpp"
#include "spdlog/common.h"
#include "vulkan/engine.hpp"

// class Vulkan_Kernels : public benchmark::Fixture {
//  public:
//   Vulkan_Kernels() {

//   }

//   Engine engine;
// };

static void RunMortonCode(benchmark::State& state) {
  //   const auto n_input = state.range(0);
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
  } pc = {static_cast<uint>(n_input),
          Config::DEFAULT_MIN_COORD,
          Config::DEFAULT_RANGE};

  auto algo = engine.algorithm("morton.spv",
                               {
                                   u_points,
                                   u_morton,
                               },
                               768,
                               reinterpret_cast<const std::byte*>(&pc),
                               sizeof(pc));

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

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::off);
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}