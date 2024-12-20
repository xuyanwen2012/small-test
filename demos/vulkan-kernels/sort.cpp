#include <spdlog/spdlog.h>

#include <algorithm>

#include "vulkan/engine.hpp"

struct PushConstants {
  uint32_t n_logical_blocks;
  uint32_t n;
  uint32_t width;
  uint32_t num_pairs;
};

int main(int argc, char** argv) {
  int n_physical_blocks = 1;
  if (argc > 1) {
    n_physical_blocks = std::atoi(argv[1]);
  }

  spdlog::set_level(spdlog::level::info);

  Engine engine;

  constexpr auto n_input = 640 * 480;

  // ---------------------------------------------------------------------------
  // Prepare the buffers
  // ---------------------------------------------------------------------------

  auto u_input_buf = engine.typed_buffer<uint32_t>(n_input);
  u_input_buf->random(1, n_input);

  auto u_output_buf = engine.typed_buffer<uint32_t>(n_input);
  u_output_buf->zeros();

  // ---------------------------------------------------------------------------
  // Run the kernel
  // ---------------------------------------------------------------------------

  auto algorithm = engine.algorithm(
      "merge_sort.spv", {u_input_buf, u_output_buf}, sizeof(PushConstants));
  auto seq = engine.sequence();

  // ---------------------------------------------------------------------------
  constexpr auto threads_per_block = 256;

  for (int width = 1; width < n_input; width *= 2) {
    int num_pairs = (n_input + 2 * width - 1) / (2 * width);
    int total_threads = num_pairs;
    int logical_blocks =
        (total_threads + threads_per_block - 1) / threads_per_block;

    PushConstants pc = {uint32_t(logical_blocks),
                        uint32_t(n_input),
                        uint32_t(width),
                        uint32_t(num_pairs)};
    algorithm->set_push_constants(pc);

    spdlog::info("blocks: {}", logical_blocks);

    // should already be swapped by the previous iteration
    algorithm->update_descriptor_sets_with_buffers({u_input_buf, u_output_buf});

    seq->record_commands_with_blocks(algorithm.get(), n_physical_blocks);
    seq->launch_kernel_async();
    seq->sync();

    std::swap(u_input_buf, u_output_buf);
  }

  // ---------------------------------------------------------------------------

  bool is_sorted = std::is_sorted(u_input_buf->begin(), u_input_buf->end());
  spdlog::info("is_sorted: {}", is_sorted);

  bool is_sorted_alt =
      std::is_sorted(u_output_buf->begin(), u_output_buf->end());
  spdlog::info("is_sorted_alt: {}", is_sorted_alt);

  assert(is_sorted != is_sorted_alt);

  return 0;
}