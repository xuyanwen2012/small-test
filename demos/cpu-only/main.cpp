#include <iostream>
#include <memory>
#include <random>
#include <vector>

#include "core/thread_pool.hpp"
// #include "host_dispatcher.hpp"

#include "memory_region.hpp"
// #include "shared/structures.h"

#include "structures.hpp"
#include "third-party/CLI11.hpp"
#include "utils.hpp"

// Problem size
constexpr auto n = 640 * 480;  // ~300k
// constexpr auto n = 1920 * 1080;  // ~2M
constexpr auto min_coord = 0.0f;
constexpr auto range = 1024.0f;
constexpr auto seed = 114514;

// demo config
constexpr auto n_iterations = 30;

// void gen_data(const std::unique_ptr<struct pipe>& p) {
//   std::mt19937 gen(seed);  // NOLINT(cert-msc51-cpp)
//   std::uniform_real_distribution dis(min_coord, min_coord + range);
//   std::generate_n(p->u_points, n, [&dis, &gen] {
//     return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
//   });
// }

int main(int argc, char** argv) {
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

  // auto p = std::make_unique<struct pipe>(n, min_coord, range, seed);
  // gen_data(p);

  // // print 10 points
  // for (int i = 0; i < 10; ++i) {
  //   std::cout << p->u_points[i].x << ' ' << p->u_points[i].y << ' '
  //             << p->u_points[i].z << '\n';
  // }

  // cpu::dispatch_morton_code(pool, 4, p).wait();

  // // print 10 morton code
  // for (int i = 0; i < 10; ++i) {
  //   std::cout << p->u_morton[i] << '\n';
  // }

  // pipe p(n, min_coord, range, seed, Backend::CPU);

  return EXIT_SUCCESS;
}
