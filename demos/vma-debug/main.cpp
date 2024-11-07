
#include <spdlog/spdlog.h>

#include "engine.hpp"

#include "shader_loader.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  {
    Engine engine;

    auto buf = engine.buffer(1024);
    auto buf2 = engine.buffer(1024);
    auto buf3 = engine.buffer(1024);
  }

  auto shader = load_shader_from_file("init.spv");
  auto shader2 = load_shader_from_file("morton.spv");

  
  return 0;
}
