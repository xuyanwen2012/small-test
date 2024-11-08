#include <spdlog/spdlog.h>

#include "algorithm.hpp"
#include "engine.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  {
    Engine engine;

    constexpr auto n = 1024;

    auto buf = engine.buffer(n * sizeof(int));
    auto buf2 = engine.buffer(n * sizeof(int));
    auto buf3 = engine.buffer(n * sizeof(int));

    constexpr auto threads_per_block = 32;

    Algorithm algo(engine.get_device(),
                   "test.spv",
                   {buf, buf2, buf3},
                   threads_per_block,
                   {n});
  }

  return 0;
}
