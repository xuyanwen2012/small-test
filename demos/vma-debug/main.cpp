
#include <spdlog/spdlog.h>

#include "base_engine.hpp"
#include "buffer.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  BaseEngine engine;
  Buffer buffer(engine.get_device(), 1024);

  Buffer buffer2(engine.get_device(), 1024);

  return 0;
}
