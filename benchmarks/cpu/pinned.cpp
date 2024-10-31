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
  static std::vector<int> small_cores_to_pin;
  static std::optional<std::vector<int>> medium_cores_to_pin;
  static std::optional<std::vector<int>> big_cores_opt_to_pin;

  explicit CPU_Pinned()
      : p(std::make_shared<Pipe>(Config::DEFAULT_N,
                                 Config::DEFAULT_MIN_COORD,
                                 Config::DEFAULT_RANGE,
                                 Config::DEFAULT_SEED)) {
    gen_data(p, Config::DEFAULT_SEED);

    // this pool only used for pregenerating the data
    int n_max_threads = std::thread::hardware_concurrency();
    core::thread_pool unpinned_pool{n_max_threads};

    cpu::dispatch_MortonCode(unpinned_pool, n_max_threads, p);
    cpu::dispatch_RadixSort(unpinned_pool, n_max_threads, p);
    cpu::dispatch_RemoveDuplicates(unpinned_pool, n_max_threads, p);
    cpu::dispatch_BuildRadixTree(unpinned_pool, n_max_threads, p);
    cpu::dispatch_EdgeCount(unpinned_pool, n_max_threads, p);
    cpu::dispatch_EdgeOffset(unpinned_pool, n_max_threads, p);
    cpu::dispatch_BuildOctree(unpinned_pool, n_max_threads, p);
  }

  std::shared_ptr<Pipe> p;
};

// Add this line after the class definition, at global scope
std::vector<int> CPU_Pinned::small_cores_to_pin;
std::optional<std::vector<int>> CPU_Pinned::medium_cores_to_pin;
std::optional<std::vector<int>> CPU_Pinned::big_cores_opt_to_pin;

// ----------------------------------------------------------------------------
// Morton code
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Pinned, BM_Morton)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  core::thread_pool pinned_pool(small_cores_to_pin, true);

  for (auto _ : state) {
    cpu::dispatch_MortonCode(pinned_pool, n_threads, p);
  }
}

void RegisterBenchmarkWithRange(int n_small_cores,
                                int n_medium_cores,
                                int n_big_cores) {
  // small cores
  for (int i = 1; i <= n_small_cores; ++i) {
    ::benchmark ::internal ::RegisterBenchmarkInternal(
        new CPU_Pinned_BM_Morton_Benchmark())
        ->Arg(i)
        ->Name("CPU_Pinned/BM_Morton/Small")
        ->Unit(benchmark::kMillisecond)
        ->Iterations(Config::DEFAULT_ITERATIONS);
  }

  // medium cores
  for (int i = 1; i <= n_medium_cores; ++i) {
    ::benchmark ::internal ::RegisterBenchmarkInternal(
        new CPU_Pinned_BM_Morton_Benchmark())
        ->Arg(i)
        ->Name("CPU_Pinned/BM_Morton/Medium")
        ->Unit(benchmark::kMillisecond)
        ->Iterations(Config::DEFAULT_ITERATIONS);
  }

  // big cores
  for (int i = 1; i <= n_big_cores; ++i) {
    ::benchmark ::internal ::RegisterBenchmarkInternal(
        new CPU_Pinned_BM_Morton_Benchmark())
        ->Arg(i)
        ->Name("CPU_Pinned/BM_Morton/Big")
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
  auto medium_cores = phone_specs.value()->mid_cores;
  auto big_cores = phone_specs.value()->big_cores;

  if (small_cores.empty()) {
    std::cerr << "No small cores found" << std::endl;
    return 1;
  }

  auto valid_small_cores = utils::get_valid_cores(small_cores);
  if (valid_small_cores.empty()) {
    std::cerr << "No valid small cores found" << std::endl;
    return 1;
  }

  auto valid_medium_cores = utils::get_valid_cores(medium_cores);
  if (valid_medium_cores.empty()) {
    // warning
    std::cerr << "No valid medium cores found" << std::endl;
  }

  auto valid_big_cores = utils::get_valid_cores(big_cores);
  if (valid_big_cores.empty()) {
    std::cerr << "No valid big cores found" << std::endl;
  }

  // set the cores to pin
  CPU_Pinned::small_cores_to_pin = valid_small_cores;
  CPU_Pinned::medium_cores_to_pin = valid_medium_cores;
  CPU_Pinned::big_cores_opt_to_pin = valid_big_cores;

  RegisterBenchmarkWithRange(
      small_cores.size(), valid_medium_cores.size(), valid_big_cores.size());

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
}
