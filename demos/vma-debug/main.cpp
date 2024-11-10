#include <spdlog/spdlog.h>

#include "algorithm.hpp"
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

    Algorithm algo(engine.get_device(),
                   "test.spv",
                   {buf, buf2, buf3},
                   threads_per_block,
                   {n});

    Sequence seq(engine.get_device(),
                 engine.get_queue(),
                 engine.get_compute_queue_index());

    seq.simple_record_commands(algo, n);
    seq.launch_kernel_async();

    seq.sync();

    // print the result
    int* data = buf3->map<int>();
    for (auto i = 0; i < 128; i++) {
      spdlog::info("data[{}] = {}", i, data[i]);
    }
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
