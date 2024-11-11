#include <spdlog/spdlog.h>

#include <memory>

#include "algorithm.hpp"
#include "buffer.hpp"
#include "engine.hpp"
#include "sequence.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  {
    Engine engine;

    constexpr auto n = 1024;

    auto buf = engine.buffer(n * sizeof(int));
    auto buf2 = engine.buffer(n * sizeof(int));
    auto buf3 = engine.buffer(n * sizeof(int));

    std::ranges::fill(buf->span<int>(), 1);
    std::ranges::fill(buf2->span<int>(), 2);
    std::ranges::fill(buf3->span<int>(), 0);

    constexpr auto threads_per_block = 32;

    auto algo =
        engine.algorithm("test.spv", {buf, buf2, buf3}, threads_per_block, {n});

    auto seq = engine.sequence();

    seq->simple_record_commands(algo.get(), n);
    seq->launch_kernel_async();

    // do other tasks on the CPU while the GPU is processing

    seq->sync();

    // print the first 128 results
    for (auto i : buf3->span<int>().subspan(0, 128)) {
      spdlog::info("{}", i);
    }
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
