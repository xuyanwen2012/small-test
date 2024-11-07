#include <spdlog/spdlog.h>

#include "engine.hpp"
#include "third-party/CLI11.hpp"

int main(int argc, char** argv) {
  CLI::App app("yanwen-vulkan");

  std::string device_name;
  app.add_option("-d,--device", device_name, "Device name")
      ->required()
      ->default_val("jetson");

  bool debug = false;
  app.add_flag("--debug", debug, "Enable debug mode");

  app.parse(argc, argv);

  if (debug) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  try {
    Engine engine{};

    auto a = engine.get_device_ptr();
    spdlog::debug("Creating buffer");
    Buffer buffer(a, 1024);
    spdlog::debug("Buffer created");

  } catch (const std::exception& e) {
    spdlog::error("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return 0;
}