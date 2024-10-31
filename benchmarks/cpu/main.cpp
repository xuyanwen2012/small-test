#include <benchmark/benchmark.h>

#include <glm/glm.hpp>
#include <memory>
#include <random>

#include "core/thread_pool.hpp"
#include "host/host_dispatcher.hpp"
#include "shared/structures.h"
#include "third-party/CLI11.hpp"
#include "utils.hpp"

namespace {

struct Config {
  static constexpr int DEFAULT_N = 640 * 480;  // ~300k
  static constexpr float DEFAULT_MIN_COORD = 0.0f;
  static constexpr float DEFAULT_RANGE = 1024.0f;
  static constexpr unsigned DEFAULT_SEED = 114514;
  static constexpr int DEFAULT_ITERATIONS = 30;
};

void gen_data(const std::shared_ptr<Pipe>& p, unsigned seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution dis(
      Config::DEFAULT_MIN_COORD,
      Config::DEFAULT_MIN_COORD + Config::DEFAULT_RANGE);
  std::generate_n(p->u_points, Config::DEFAULT_N, [&dis, &gen] {
    return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
  });
}

}  // namespace

class CPU : public benchmark::Fixture {
 public:
  explicit CPU()
      : p(std::make_shared<Pipe>(Config::DEFAULT_N,
                                 Config::DEFAULT_MIN_COORD,
                                 Config::DEFAULT_RANGE,
                                 Config::DEFAULT_SEED)),
        pool({0, 1, 2, 3, 4, 5}) {
    gen_data(p, Config::DEFAULT_SEED);

    const auto n_max_threads = pool.get_thread_count();

    // basically pregenerate the data
    cpu::dispatch_MortonCode(pool, n_max_threads, p);
    cpu::dispatch_RadixSort(pool, n_max_threads, p);
    cpu::dispatch_RemoveDuplicates(pool, n_max_threads, p);
    cpu::dispatch_BuildRadixTree(pool, n_max_threads, p);
    cpu::dispatch_EdgeCount(pool, n_max_threads, p);
    cpu::dispatch_EdgeOffset(pool, n_max_threads, p);
    cpu::dispatch_BuildOctree(pool, n_max_threads, p);
  }

  std::shared_ptr<Pipe> p;
  core::thread_pool pool;
};

// ----------------------------------------------------------------------------
// Morton code
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU, BM_Morton)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_MortonCode(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU, BM_Morton)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

int main(int argc, char** argv) {
  CLI::App app("Benchmark for CPU");

  std::vector<int> small_cores;
  std::vector<int> medium_cores;
  std::vector<int> big_cores;

  const auto max_cores = std::thread::hardware_concurrency();

  app.add_option("-s,--small_cores", small_cores, "Small cores to use")
      ->check(CLI::Range(0u, max_cores));

  app.add_option("-m,--medium_cores", medium_cores, "Medium cores to use")
      ->check(CLI::Range(0u, max_cores));

  app.add_option("-b,--big_cores", big_cores, "Big cores to use")
      ->check(CLI::Range(0u, max_cores));

  CLI11_PARSE(app, argc, argv);

  if (!utils::validate_cores(small_cores)) {
    std::cerr << "Small cores are not available\n";
    return EXIT_FAILURE;
  }

  if (!utils::validate_cores(medium_cores)) {
    std::cerr << "Medium cores are not available\n";
    return EXIT_FAILURE;
  }

  if (!utils::validate_cores(big_cores)) {
    std::cerr << "Big cores are not available\n";
    return EXIT_FAILURE;
  }

  // if (!is_subset(small_cores, get_available_cores())) {
  //     throw std::invalid_argument("Small cores are not available");
  // }

  ::benchmark::Initialize(&argc, argv);  // Initialize Google Benchmark

  //   if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  // ::benchmark::RunSpecifiedBenchmarks();  // Run benchmarks
}