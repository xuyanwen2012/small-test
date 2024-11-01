#include <benchmark/benchmark.h>

#include "bm_config.hpp"
#include "core/thread_pool.hpp"
#include "host/host_dispatcher.hpp"

class CPU_JustKernel : public benchmark::Fixture {
 public:
  explicit CPU_JustKernel()
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

void RegisterBenchmarks(std::span<const int> available_cores_to_pin) {
  auto p = std::make_shared<Pipe>(Config::DEFAULT_N,
                                  Config::DEFAULT_MIN_COORD,
                                  Config::DEFAULT_RANGE,
                                  Config::DEFAULT_SEED);
  gen_data(p, Config::DEFAULT_SEED);

  for (int core_id : available_cores_to_pin) {
    auto mc_benchmark = [core_id, p](benchmark::State& state) {
      utils::set_cpu_affinity(core_id);

      for (auto _ : state) {
        for (int i = 0; i < p->n_input(); ++i) {
          p->u_morton[i] =
              shared::xyz_to_morton32(p->u_points[i], p->min_coord, p->range);
        }
      }
    };
    benchmark::RegisterBenchmark(
        ("BM_JustKernel_" + std::to_string(core_id)).c_str(), mc_benchmark)
        ->Unit(benchmark::kMillisecond)
        ->Iterations(Config::DEFAULT_ITERATIONS);
  }
}

void RegisterBenchmarksWithPool(std::span<const int> available_cores_to_pin) {
  auto p = std::make_shared<Pipe>(Config::DEFAULT_N,
                                  Config::DEFAULT_MIN_COORD,
                                  Config::DEFAULT_RANGE,
                                  Config::DEFAULT_SEED);
  gen_data(p, Config::DEFAULT_SEED);

  for (int core_id : available_cores_to_pin) {
    auto mc_benchmark = [core_id, p](benchmark::State& state) {
      core::thread_pool pool(std::vector<int>{core_id}, true);
      
      for (auto _ : state) {

        pool.submit_task([p] {
          for (int i = 0; i < p->n_input(); ++i) {
            p->u_morton[i] =
                shared::xyz_to_morton32(p->u_points[i], p->min_coord, p->range);
          }
        }).wait();
      }
    };
    benchmark::RegisterBenchmark(
        ("BM_JustKernel_Pool_" + std::to_string(core_id)).c_str(), mc_benchmark)
        ->Unit(benchmark::kMillisecond)
        ->Iterations(Config::DEFAULT_ITERATIONS);
  }
}

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);

  auto available_cores_to_pin = utils::get_available_cores();

  // print available cores to pin
  std::cout << "Available cores to pin: ";
  for (int core_id : available_cores_to_pin) {
    std::cout << core_id << " ";
  }
  std::cout << std::endl;

  RegisterBenchmarks(available_cores_to_pin);
  RegisterBenchmarksWithPool(available_cores_to_pin);

  benchmark::RunSpecifiedBenchmarks();

  return 0;
}
