#include <spdlog/spdlog.h>

#include <algorithm>

#include "vulkan/engine.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 128;

  auto u_input_buf = engine.typed_buffer<uint32_t>(n_input);
  u_input_buf->ones();

  auto u_output_buf = engine.typed_buffer<uint32_t>(n_input);
  u_output_buf->fill(2);

  auto debug_output1_buf = engine.typed_buffer<uint32_t>(n_input);
  debug_output1_buf->zeros();

  auto debug_output2_buf = engine.typed_buffer<uint32_t>(n_input);
  debug_output2_buf->zeros();

  const auto algo = engine.algorithm(
      "print-128.spv",
      {u_input_buf, u_output_buf, debug_output1_buf, debug_output2_buf},
      0);
  const auto seq = engine.sequence();

  //   algo->update_descriptor_sets_with_buffers({u_input_buf, u_output_buf});
  seq->record_commands_with_blocks(algo.get(), 1);
  seq->launch_kernel_async();
  seq->sync();

  // print out the debug buffers
  for (int i = 0; i < n_input; i++) {
    spdlog::info(
        "[{}] {} {}", i, debug_output1_buf->at(i), debug_output2_buf->at(i));
  }

  algo->update_descriptor_sets_with_buffers(
      {u_output_buf, u_input_buf, debug_output1_buf, debug_output2_buf});

  seq->record_commands_with_blocks(algo.get(), 1);
  seq->launch_kernel_async();
  seq->sync();

  // print out the debug buffers
  for (int i = 0; i < n_input; i++) {
    spdlog::info(
        "[{}] {} {}", i, debug_output1_buf->at(i), debug_output2_buf->at(i));
  }

  algo->update_descriptor_sets_with_buffers(
      {u_input_buf, u_output_buf, debug_output1_buf, debug_output2_buf});
  seq->record_commands_with_blocks(algo.get(), 1);
  seq->launch_kernel_async();
  seq->sync();

  // print out the debug buffers
  for (int i = 0; i < n_input; i++) {
    spdlog::info(
        "[{}] {} {}", i, debug_output1_buf->at(i), debug_output2_buf->at(i));
  }

  return 0;
}
