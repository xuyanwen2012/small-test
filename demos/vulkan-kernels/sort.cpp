#include <spdlog/spdlog.h>

#include <algorithm>
#include <random>

#include "vulkan/engine.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 1024;
  constexpr auto n_threads = 256;  // defined in the shader
  constexpr auto n_blocks = (n_input + n_threads - 1) / n_threads;

  // ---------------------------------------------------------------------------
  // Prepare the buffers
  // ---------------------------------------------------------------------------

  auto input_buffer = engine.buffer(n_input * sizeof(uint32_t));
  input_buffer->random(1, n_input);

  // Print first 10 elements of input
  spdlog::info("Input array:");
  for (int i = 0; i < std::min(10, n_input); i++) {
    spdlog::info("Input {}: {}", i, input_buffer->as<uint32_t>()[i]);
  }

  auto output_buffer = engine.buffer(n_input * sizeof(uint32_t));
  output_buffer->zeros();

  // ---------------------------------------------------------------------------
  // Run the kernel
  // ---------------------------------------------------------------------------

  struct {
    uint32_t n;
  } pc = {n_input};

  auto algorithm = engine.algorithm("sort.spv",
                                    {input_buffer, output_buffer},
                                    n_blocks,
                                    reinterpret_cast<const std::byte*>(&pc),
                                    sizeof(pc));

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algorithm.get(), n_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // Verify results
  spdlog::info("\nSorted array:");
  for (int i = 0; i < std::min(10, n_input); i++) {
    spdlog::info("Output {}: {}", i, output_buffer->as<uint32_t>()[i]);
  }

  // Verify if sorted
  bool is_sorted = true;
  auto output_data = output_buffer->as<uint32_t>();
  for (int i = 1; i < n_input; i++) {
    if (output_data[i] < output_data[i - 1]) {
      is_sorted = false;
      spdlog::error("Array not sorted at position {}: {} > {}",
                    i,
                    output_data[i - 1] ,
                    output_data[i]);
      break;
    }
  }

  spdlog::info("Array is {}", is_sorted ? "sorted" : "not sorted");

  return 0;
}