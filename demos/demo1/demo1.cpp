#include <iostream>
#include <mutex>
#include <vector>

#include "core/thread_pool.hpp"
#include "third-party/CLI11.hpp"
#include "utils.hpp"

int main(int argc, char **argv) {
  CLI::App app{"CPU demo of using all threads"};

  std::vector<int> small_cores;

  app.add_option("-s,--small_cores", small_cores, "Small cores")
      ->default_val(std::vector<int>{0, 1, 2, 3})
      ->check(CLI::Range(0, 8));

  CLI11_PARSE(app, argc, argv);

  utils::display_core_info();

  if (!utils::validate_cores(small_cores)) {
    return EXIT_FAILURE;
  }

  // print cores used
  std::cout << "Using cores: ";
  std::ranges::copy(small_cores, std::ostream_iterator<int>(std::cout, " "));
  std::cout << '\n';

  core::thread_pool pool(small_cores);

  std::mutex mtx;
  pool.submit_blocks(0,
                     123,
                     [&](const auto start, const auto end) {
                       utils::busy_wait_for(std::chrono::seconds(2));
                       {
                         std::lock_guard lock(mtx);
                         std::cout << "Processing block from " << start
                                   << " to " << end << '\n';
                       }
                     })
      .wait();

  return EXIT_SUCCESS;
}
