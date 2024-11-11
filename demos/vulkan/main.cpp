#include <spdlog/spdlog.h>

#include <glm/glm.hpp>
#include <memory>

#include "glm_helper.hpp"
#include "third-party/CLI11.hpp"
#include "vulkan/engine.hpp"

int main(int argc, char** argv) {
  CLI::App app{"VMA Debug"};

  bool debug = false;
  app.add_flag("--debug", debug, "Enable debug mode");
  app.allow_extras();

  CLI11_PARSE(app, argc, argv);

  spdlog::set_level(debug ? spdlog::level::debug : spdlog::level::info);

  Engine engine;

  constexpr auto n = 1024;
  constexpr auto min_val = 0;
  constexpr auto range = 1024;
  constexpr auto seed = 114514;

  // (1) init kernel
  {
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

    auto buf = engine.buffer(n * sizeof(glm::vec4));

    auto algo =
        engine.algorithm("init.spv", {buf}, 512, {n, min_val, range, seed});

    auto seq = engine.sequence();
    seq->simple_record_commands(algo.get(), n);
    seq->launch_kernel_async();
    seq->sync();

    // print the first 10 elements
    auto span = buf->span<glm::vec4>();
    for (auto i = 0; i < 10; ++i) {
      spdlog::info("{}: {}", i, span[i]);
    }
  }

  spdlog::info("Done!");
  return EXIT_SUCCESS;
}
