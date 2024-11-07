#include <spdlog/spdlog.h>

#include "base_engine.hpp"
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
    BaseEngine engine;
  } catch (const std::exception& e) {
    spdlog::error("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return 0;
}