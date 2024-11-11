#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <memory>

#include "glm_helper.hpp"
#include "third-party/CLI11.hpp"
#include "vulkan/engine.hpp"

void test_init_kernel(Engine& engine, int num_blocks) {
  // layout(set = 0, binding = 0) buffer RandomBuffer { vec4 random_buffer[];
  // };

  // layout(push_constant) uniform PushConstant {
  //   int size;
  //   int min_val;
  //   int range;
  //   int seed;
  // };
  //
  // layout(local_size_x = 512) in;

  constexpr int n = 1024;
  constexpr int min_val = 0;
  constexpr int range = 1;
  constexpr int seed = 114514;

  auto buf = engine.buffer(n * sizeof(glm::vec4));
  buf->zeros();

  auto algo =
      engine.algorithm<int>("init.spv", {buf}, 512, {n, min_val, range, seed});

  auto seq = engine.sequence();

  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // print results
  auto span = buf->span<glm::vec4>();
  for (auto i = 0; i < 10; ++i) {
    spdlog::info("{}: {}", i, span[i]);
  }
}

void test_morton_kernel(Engine& engine, int num_blocks) {
  // layout(set = 0, binding = 0) buffer Data { vec4 data[]; };

  // layout(set = 0, binding = 1) buffer MortonKeys { uint morton_keys[]; };

  // layout(push_constant) uniform Constants {
  //   uint n;
  //   float min_coord;
  //   float range;
  // };

  // layout(local_size_x = 768) in;

  constexpr int n = 1024;
  constexpr float min_coord = 0.0f;
  constexpr float range = 1.0f;

  auto in_buf = engine.buffer(n * sizeof(glm::vec4));
  for (auto i = 0; i < n; ++i) {
    in_buf->map<glm::vec4>()[i] =
        glm::vec4(0.01f * i, 0.01f * i, 0.01f * i, 0.01f * i);
  }

  auto morton_buf = engine.buffer(n * sizeof(uint32_t));
  morton_buf->zeros();

  auto algo = engine.algorithm<float>(
      "morton.spv", {in_buf, morton_buf}, 768, {n, min_coord, range});

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // print 10 results
  auto morton_span = morton_buf->span<uint32_t>();
  for (auto i = 0; i < 10; ++i) {
    spdlog::info("Morton code {}: {}", i, morton_span[i]);
  }
}

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

  constexpr int n = 1024;
  constexpr int min_val = 0;
  constexpr int range = 1;
  constexpr int seed = 114514;

  // print all parameters and configs
  spdlog::info("n: {}", n);
  spdlog::info("min_val: {}", min_val);
  spdlog::info("range: {}", range);
  spdlog::info("seed: {}", seed);
  spdlog::info("num_blocks: {}", num_blocks);

  test_init_kernel(engine, num_blocks);
  test_morton_kernel(engine, num_blocks);

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
