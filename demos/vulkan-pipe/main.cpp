#include <spdlog/spdlog.h>

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

#include "glm_helper.hpp"
#include "shared/morton_func.h"
#include "third-party/CLI11.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/engine.hpp"

// struct Pipe {
//   // ------------------------
//   // Essential Data (CPU/GPU shared)
//   // ------------------------

//   // mutable
//   int n_unique = UNINITIALIZED;

//   glm::vec4* u_points;
//   morton_t* u_morton;
//   morton_t* u_morton_alt;  // also used as the unique morton
//   // RadixTree brt;
//   int* u_edge_counts;
//   int* u_edge_offsets;
//   // Octree oct;

//   // read-only
//   int n_points;
//   float min_coord;
//   float range;
//   int seed;
// };

struct Pipe {
  // ------------------------
  // Essential Data (CPU/GPU shared)
  // ------------------------
  Pipe(Engine& engine,
       const int n_points,
       const float min_coord,
       const float range,
       const int seed,
       const int num_blocks)
      : n_points(n_points),
        min_coord(min_coord),
        range(range),
        seed(seed),
        num_blocks(num_blocks),
        engine(engine) {
    u_points = engine.buffer(n_points * sizeof(glm::vec4));
    u_morton = engine.buffer(n_points * sizeof(morton_t));
    u_morton_alt = engine.buffer(n_points * sizeof(morton_t));
    u_edge_counts = engine.buffer(n_points * sizeof(int));
    u_edge_offsets = engine.buffer(n_points * sizeof(int));

    seq = engine.sequence();
  }

  // mutable
  int n_unique = UNINITIALIZED;

  std::shared_ptr<Buffer> u_points;
  std::shared_ptr<Buffer> u_morton;
  std::shared_ptr<Buffer> u_morton_alt;
  std::shared_ptr<Buffer> u_edge_counts;
  std::shared_ptr<Buffer> u_edge_offsets;

  // read-only
  const int n_points;
  const float min_coord;
  const float range;
  const int seed;

  const int num_blocks;

  Engine& engine;
  std::shared_ptr<Sequence> seq;

  // ---------------------------------------------------------------------------
  // Methods
  // ---------------------------------------------------------------------------

  void init() {
    struct {
      int n;
      int min_val;
      int range;
      int seed;
    } init_push_constants = {
        n_points, static_cast<int>(min_coord), static_cast<int>(range), seed};

    auto algo = engine.algorithm(
        "init.spv",
        {u_points},
        512,
        reinterpret_cast<const std::byte*>(&init_push_constants),
        sizeof(init_push_constants));

    seq->record_commands_with_blocks(algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();
  }

  void compute_morton() {
    struct {
      int n;
      float min_coord;
      float range;
    } compute_morton_push_constants = {n_points, min_coord, range};

    auto algo = engine.algorithm(
        "morton.spv",
        {u_points, u_morton},
        512,
        reinterpret_cast<const std::byte*>(&compute_morton_push_constants),
        sizeof(compute_morton_push_constants));

    seq->record_commands_with_blocks(algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();
  }

  void sort_morton() {
    std::ranges::sort(u_morton->span<morton_t>());
    std::ranges::copy(u_morton->span<morton_t>(),
                      u_morton_alt->map<morton_t>());
  }

  void remove_duplicates() {
    n_unique = std::unique(u_morton_alt->span<morton_t>().begin(),
                           u_morton_alt->span<morton_t>().end()) -
               u_morton_alt->span<morton_t>().begin();

    spdlog::info("n_unique: {}", n_unique);
  }
};

int main(int argc, char** argv) {
  CLI::App app{"VMA Debug"};

  // parameters
  bool debug = false;
  int num_blocks = 1;

  app.add_flag("--debug", debug, "Enable debug mode");
  app.add_option("-b,--blocks", num_blocks, "Number of blocks")->default_val(1);
  app.allow_extras();

  CLI11_PARSE(app, argc, argv);

  spdlog::set_level(debug ? spdlog::level::debug : spdlog::level::info);

  Engine engine;

  // print all parameters and configs

  spdlog::info("num_blocks: {}", num_blocks);

  Pipe pipe{engine, 1024, 0.0f, 1.0f, 114514, num_blocks};

  pipe.init();
  pipe.compute_morton();
  pipe.sort_morton();

  // check if the morton codes are sorted
  auto morton_span = pipe.u_morton_alt->span<morton_t>();
  assert(std::ranges::is_sorted(morton_span));

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
