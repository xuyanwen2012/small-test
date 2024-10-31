#include <benchmark/benchmark.h>

#include <glm/glm.hpp>

#include "bm_config.hpp"
#include "core/thread_pool.hpp"
#include "host/host_dispatcher.hpp"
#include "shared/structures.h"

// ----------------------------------------------------------------------------
// Unpinned benchmark
// ----------------------------------------------------------------------------

class CPU_Unpined : public benchmark::Fixture {
 public:
  explicit CPU_Unpined()
      : p(std::make_shared<Pipe>(Config::DEFAULT_N,
                                 Config::DEFAULT_MIN_COORD,
                                 Config::DEFAULT_RANGE,
                                 Config::DEFAULT_SEED)),
        pool(std::thread::hardware_concurrency()) {
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

BENCHMARK_DEFINE_F(CPU_Unpined, BM_Morton)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_MortonCode(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_Morton)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Radix sort
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_Sort)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_RadixSort(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_Sort)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Remove duplicates
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_RemoveDuplicates)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_RemoveDuplicates(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_RemoveDuplicates)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Build radix tree
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_BuildRadixTree)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_BuildRadixTree(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_BuildRadixTree)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Edge count
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_EdgeCount)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_EdgeCount(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_EdgeCount)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Edge offset
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_EdgeOffset)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_EdgeOffset(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_EdgeOffset)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

// ----------------------------------------------------------------------------
// Build octree
// ----------------------------------------------------------------------------

BENCHMARK_DEFINE_F(CPU_Unpined, BM_BuildOctree)(benchmark::State& state) {
  const auto n_threads = state.range(0);

  for (auto _ : state) {
    cpu::dispatch_BuildOctree(pool, n_threads, p);
  }
}

BENCHMARK_REGISTER_F(CPU_Unpined, BM_BuildOctree)
    ->DenseRange(1, std::thread::hardware_concurrency(), 1)
    ->Unit(benchmark::kMillisecond)
    ->Iterations(Config::DEFAULT_ITERATIONS);

int main(int argc, char** argv) {
  // ignore the command line arguments from "--device=<device>"

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  return 0;
}
