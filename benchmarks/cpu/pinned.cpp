#include <benchmark/benchmark.h>

#include <glm/glm.hpp>
#include <memory>
#include <random>

#include "bm_config.hpp"
#include "configs.hpp"
#include "core/thread_pool.hpp"
#include "host/host_dispatcher.hpp"
#include "shared/structures.h"
#include "third-party/CLI11.hpp"
#include "utils.hpp"

class CPU_Pinned : public benchmark::Fixture {
 public:
  // all phones has small cores
  static std::vector<int> small_cores_opt;
  static std::optional<std::vector<int>> medium_cores_opt;
  static std::optional<std::vector<int>> big_cores_opt;

  explicit CPU_Pinned()
      : p(std::make_shared<Pipe>(Config::DEFAULT_N,
                                 Config::DEFAULT_MIN_COORD,
                                 Config::DEFAULT_RANGE,
                                 Config::DEFAULT_SEED)) {
    gen_data(p, Config::DEFAULT_SEED);

    int n_max_threads = std::thread::hardware_concurrency();
    core::thread_pool unpinned_pool{n_max_threads};

    // basically pregenerate the data
    cpu::dispatch_MortonCode(unpinned_pool, n_max_threads, p);
    cpu::dispatch_RadixSort(unpinned_pool, n_max_threads, p);
    cpu::dispatch_RemoveDuplicates(unpinned_pool, n_max_threads, p);
    cpu::dispatch_BuildRadixTree(unpinned_pool, n_max_threads, p);
    cpu::dispatch_EdgeCount(unpinned_pool, n_max_threads, p);
    cpu::dispatch_EdgeOffset(unpinned_pool, n_max_threads, p);
    cpu::dispatch_BuildOctree(unpinned_pool, n_max_threads, p);
  }

  std::shared_ptr<Pipe> p;

  // std::vector<int> small_cores;
  // std::optional<std::vector<int>> small_cores_opt;
};

// Add this line after the class definition, at global scope
std::vector<int> CPU_Pinned::small_cores_opt;
std::optional<std::vector<int>> CPU_Pinned::medium_cores_opt;
std::optional<std::vector<int>> CPU_Pinned::big_cores_opt;

// ----------------------------------------------------------------------------
// Morton code
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Pinned, BM_Morton)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  core::thread_pool pinned_pool(small_cores_opt, true);

  for (auto _ : state) {
    cpu::dispatch_MortonCode(pinned_pool, n_threads, p);
  }
}

void RegisterBenchmarkWithRange(int max_threads) {
  for (int i = 1; i <= max_threads; ++i) {
    ::benchmark ::internal ::RegisterBenchmarkInternal(
        new CPU_Pinned_BM_Morton_Benchmark())
        ->Arg(i)
        ->Unit(benchmark::kMillisecond)
        ->Iterations(Config::DEFAULT_ITERATIONS);
  }
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------

int main(int argc, char** argv) {
  CLI::App app("Pinned benchmark");

  std::string device;
  app.add_option("--device", device, "Device ID")->default_val("jetson");
  CLI11_PARSE(app, argc, argv);

  auto phone_specs = get_phone_specs(device);
  if (!phone_specs) {
    std::cerr << "Failed to get phone specs" << std::endl;
    return 1;
  }

  auto small_cores = phone_specs.value()->small_cores;
  if (small_cores.empty()) {
    std::cerr << "No small cores found" << std::endl;
    return 1;
  }

  // set small_cores_opt
  CPU_Pinned::small_cores_opt = small_cores;

  // CPU_Pinned BM_Morton_instance;
  // print size of small_cores
  RegisterBenchmarkWithRange(small_cores.size());

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
