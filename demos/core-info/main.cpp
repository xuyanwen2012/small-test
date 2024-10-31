#include <iomanip>

#include "configs.hpp"
#include "third-party/CLI11.hpp"
#include "utils.hpp"

int main(int argc, char** argv) {
  CLI::App app{"CPU demo of using all threads"};

  std::string device_id;
  app.add_option("--device", device_id, "Device ID")->default_val("jetson");

  CLI11_PARSE(app, argc, argv);

  utils::display_core_info();

  auto phone_specs = get_phone_specs(device_id);
  if (!phone_specs) {
    std::cerr << "Error: Invalid device ID: " << device_id << std::endl;
    return EXIT_FAILURE;
  }

  utils::print_device_profile(phone_specs.value());

  return EXIT_SUCCESS;
}
