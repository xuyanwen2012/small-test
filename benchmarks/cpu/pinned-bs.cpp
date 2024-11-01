#include <benchmark/benchmark.h>

#include <glm/glm.hpp>
#include <memory>
#include <random>

#include "bm_config.hpp"
#include "configs.hpp"
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

// Add helper function to get appropriate cores for the benchmark
static std::vector<int> get_cores_for_type(const std::string& core_type) {
  if (core_type == "Small") {
    return CPU_Pinned::small_cores_to_pin;
  } else if (core_type == "Medium" && CPU_Pinned::medium_cores_to_pin) {
    return CPU_Pinned::medium_cores_to_pin.value();
  } else if (core_type == "Big" && CPU_Pinned::big_cores_opt_to_pin) {
    return CPU_Pinned::big_cores_opt_to_pin.value();
  }
  return std::vector<int>();
}

// Modify the benchmark macro to include core type
#define DEFINE_PINNED_BENCHMARK(NAME)                                    \
  BENCHMARK_DEFINE_F(CPU_Pinned, NAME)(benchmark::State & state) {       \
    const auto n_threads = state.range(0);                               \
    std::string core_type = state.name();                                \
    core_type = core_type.substr(core_type.rfind('/') + 1);              \
    auto cores_to_pin = get_cores_for_type(core_type);                   \
    if (cores_to_pin.empty()) {                                          \
      state.SkipWithError("No valid cores found for type " + core_type); \
      return;                                                            \
    }                                                                    \
    core::thread_pool pinned_pool(cores_to_pin, true);                   \
    for (auto _ : state) {                                               \
      cpu::dispatch_##NAME(pinned_pool, n_threads, p);                   \
    }                                                                    \
  }

// Replace all individual benchmark definitions with the macro
DEFINE_PINNED_BENCHMARK(MortonCode)
DEFINE_PINNED_BENCHMARK(RadixSort)
DEFINE_PINNED_BENCHMARK(RemoveDuplicates)
DEFINE_PINNED_BENCHMARK(BuildRadixTree)
DEFINE_PINNED_BENCHMARK(EdgeCount)
DEFINE_PINNED_BENCHMARK(EdgeOffset)
DEFINE_PINNED_BENCHMARK(BuildOctree)

#undef DEFINE_PINNED_BENCHMARK

// Modify RegisterBenchmarkWithRange to handle empty core sets
void RegisterBenchmarkWithRange(int n_small_cores,
                                int n_medium_cores,
                                int n_big_cores) {
#define REGISTER_BENCHMARK(NAME, CORE_TYPE, N_CORES)    \
  if (N_CORES > 0) {                                    \
    for (int i = 1; i <= N_CORES; ++i) {                \
      ::benchmark::internal::RegisterBenchmarkInternal( \
          new CPU_Pinned_##NAME##_Benchmark())          \
          ->Arg(i)                                      \
          ->Name("CPU_Pinned/" #NAME "/" #CORE_TYPE)    \
          ->Unit(benchmark::kMillisecond)               \
          ->Iterations(Config::DEFAULT_ITERATIONS);     \
    }                                                   \
  }

  REGISTER_BENCHMARK(MortonCode, Small, n_small_cores);
  REGISTER_BENCHMARK(MortonCode, Medium, n_medium_cores);
  REGISTER_BENCHMARK(MortonCode, Big, n_big_cores);

  // REGISTER_BENCHMARK(RadixSort, Small, n_small_cores);
  // REGISTER_BENCHMARK(RadixSort, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(RadixSort, Big, n_big_cores);

  // REGISTER_BENCHMARK(RemoveDuplicates, Small, n_small_cores);
  // REGISTER_BENCHMARK(RemoveDuplicates, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(RemoveDuplicates, Big, n_big_cores);

  // REGISTER_BENCHMARK(BuildRadixTree, Small, n_small_cores);
  // REGISTER_BENCHMARK(BuildRadixTree, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(BuildRadixTree, Big, n_big_cores);

  // REGISTER_BENCHMARK(EdgeCount, Small, n_small_cores);
  // REGISTER_BENCHMARK(EdgeCount, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(EdgeCount, Big, n_big_cores);

  // REGISTER_BENCHMARK(EdgeOffset, Small, n_small_cores);
  // REGISTER_BENCHMARK(EdgeOffset, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(EdgeOffset, Big, n_big_cores);

  // REGISTER_BENCHMARK(BuildOctree, Small, n_small_cores);
  // REGISTER_BENCHMARK(BuildOctree, Medium, n_medium_cores);
  // REGISTER_BENCHMARK(BuildOctree, Big, n_big_cores);

#undef REGISTER_BENCHMARK
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
