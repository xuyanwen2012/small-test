#include <spdlog/spdlog.h>

#include "algorithm.hpp"
#include "engine.hpp"
#include "shader_loader.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  {
    Engine engine;

    auto buf = engine.buffer(1024);
    auto buf2 = engine.buffer(1024);
    auto buf3 = engine.buffer(1024);

    Algorithm algo(engine.get_device(), "init.spv", {buf, buf2, buf3}, 1024);
  }

  return 0;
}
