#include <spdlog/spdlog.h>

#include <cstdint>
#include <glm/glm.hpp>
#include <memory>

#include "glm_helper.hpp"
#include "shared/morton_func.h"
#include "third-party/CLI11.hpp"
#include "vulkan/buffer.hpp"
#include "vulkan/engine.hpp"

// // ------------------------
// // Essential Data
// // ------------------------
// int n_brt_nodes = UNINITIALIZED;

// uint8_t* u_prefix_n;
// bool* u_has_leaf_left;
// bool* u_has_leaf_right;
// int* u_left_child;
// int* u_parents;
struct RadixTree {
  RadixTree() = delete;

  explicit RadixTree(Engine& engine, int n_brt_nodes)
      : n_brt_nodes(n_brt_nodes), engine(engine) {
    u_prefix_n = engine.buffer(n_brt_nodes * sizeof(uint8_t));
    u_has_leaf_left = engine.buffer(n_brt_nodes * sizeof(bool));
    u_has_leaf_right = engine.buffer(n_brt_nodes * sizeof(bool));
    u_left_child = engine.buffer(n_brt_nodes * sizeof(int));
    u_parents = engine.buffer(n_brt_nodes * sizeof(int));
  }

  int n_brt_nodes;

  std::shared_ptr<Buffer> u_prefix_n;
  std::shared_ptr<Buffer> u_has_leaf_left;
  std::shared_ptr<Buffer> u_has_leaf_right;
  std::shared_ptr<Buffer> u_left_child;
  std::shared_ptr<Buffer> u_parents;

  Engine& engine;
};

struct Pipe {
  Pipe() = delete;

  explicit Pipe(Engine& engine,
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

  // RadixTree brt;
  std::unique_ptr<RadixTree> brt;

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

  void allocate_brt() {
    if (n_unique == UNINITIALIZED) {
      throw std::runtime_error("n_unique is UNINITIALIZED");
    }

    brt = std::make_unique<RadixTree>(engine, n_unique - 1);
  }

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

  void build_radix_tree() {
    struct {
      int n_unique;
    } build_radix_tree_push_constants = {n_unique};

    auto algo = engine.algorithm(
        "build_radix_tree.spv",
        {u_morton_alt,
         brt->u_prefix_n,
         brt->u_has_leaf_left,
         brt->u_has_leaf_right,
         brt->u_left_child,
         brt->u_parents},
        256,
        reinterpret_cast<const std::byte*>(&build_radix_tree_push_constants),
        sizeof(build_radix_tree_push_constants));

    seq->record_commands_with_blocks(algo.get(), num_blocks);
    seq->launch_kernel_async();
    seq->sync();
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
  pipe.remove_duplicates();

  // check if the morton codes are sorted
  // auto morton_span = pipe.u_morton_alt->span<morton_t>();
  assert(std::ranges::is_sorted(morton_span));

  pipe.allocate_brt();
  pipe.build_radix_tree();

  // print the radix tree first 10 nodes
  auto prefix_n_span = pipe.brt->u_prefix_n->span<uint8_t>();
  auto has_leaf_left_span = pipe.brt->u_has_leaf_left->span<bool>();
  auto has_leaf_right_span = pipe.brt->u_has_leaf_right->span<bool>();
  auto left_child_span = pipe.brt->u_left_child->span<int>();
  auto parents_span = pipe.brt->u_parents->span<int>();

  for (int i = 0; i < 10; ++i) {
    spdlog::info("Node {}:", i);
    spdlog::info("\tprefix_n: {}", prefix_n_span[i]);
    spdlog::info("\thas_leaf_left: {}", has_leaf_left_span[i]);
    spdlog::info("\thas_leaf_right: {}", has_leaf_right_span[i]);
    spdlog::info("\tleft_child: {}", left_child_span[i]);
    spdlog::info("\tparents: {}", parents_span[i]);
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
