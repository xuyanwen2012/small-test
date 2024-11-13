#include <spdlog/spdlog.h>

#include <algorithm>
#include <random>

#include "vulkan/engine.hpp"
#include "vulkan/typed_buffer.hpp"
int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 1024;
  constexpr auto n_threads = 256;  // defined in the shader
  constexpr auto n_blocks = (n_input + n_threads - 1) / n_threads;

  // ---------------------------------------------------------------------------
  // Prepare the buffers
  // ---------------------------------------------------------------------------

  auto u_input_buf = engine.typed_buffer<uint32_t>(n_input);
  u_input_buf->random(1, n_input);

  auto u_output_buf = engine.typed_buffer<uint32_t>(n_input);
  u_output_buf->zeros();

  constexpr auto BASE = 256;
  auto u_shared_bucket_buf = engine.typed_buffer<uint32_t>(BASE);
  u_shared_bucket_buf->zeros();

  // ---------------------------------------------------------------------------
  // Run the kernel
  // ---------------------------------------------------------------------------

  struct {
    uint32_t n;
    uint32_t shift;
  } pc = {n_input, 0};

  auto algorithm =
      engine.algorithm("digit_binning.spv",
                       {u_input_buf, u_output_buf, u_shared_bucket_buf},
                       256,
                       reinterpret_cast<const std::byte*>(&pc),
                       sizeof(pc));

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algorithm.get(), n_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // print histogram
  spdlog::info("Histogram:");
  for (int i = 0; i < BASE; i++) {
    spdlog::info("Histogram {}: {}", i, u_shared_bucket_buf->at(i));
  }

  return 0;
}