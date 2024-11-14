#include <spdlog/spdlog.h>

#include <algorithm>
#include <random>

#include "vulkan/engine.hpp"
#include "vulkan/typed_buffer.hpp"

struct PushConstants {
  uint32_t n_logical_blocks;
  uint32_t n;
  uint32_t width;
  uint32_t num_pairs;
};

void gpu_merge_sort(Algorithm *algorithm,
                    Sequence *seq,
                    int n_physical_blocks,
                    // uint32_t *data,
                    // uint32_t *temp,
                    TypedBuffer<uint32_t> *u_input_buf,
                    TypedBuffer<uint32_t> *u_output_buf,
                    int n) {
  // uint32_t *input = data;
  // uint32_t *output = temp;

  constexpr auto threads_per_block = 256;

  for (int width = 1; width < n; width *= 2) {
    int num_pairs = (n + 2 * width - 1) / (2 * width);
    int total_threads = num_pairs;
    int logical_blocks =
        (total_threads + threads_per_block - 1) / threads_per_block;

    PushConstants pc = {uint32_t(logical_blocks),
                        uint32_t(n),
                        uint32_t(width),
                        uint32_t(num_pairs)};
    algorithm->set_push_constants(pc);

    seq->record_commands_with_blocks(algorithm, n_physical_blocks);
    seq->launch_kernel_async();
    seq->sync();

    // swap input and output
    std::swap(u_input_buf, u_output_buf);

    // merge_kernel<<<n_desired_blocks, threads_per_block>>>(
    //     blocks, input, output, n, width, num_pairs);
    // cudaDeviceSynchronize();

    // // Swap input and output pointers for next iteration
    // uint32_t *temp_ptr = input;
    // input = output;
    // output = temp_ptr;
  }

  // // If the final output is in the temp array, copy it back to data
  // if (input != data) {
  //   cudaMemcpy(data, input, n * sizeof(uint32_t), cudaMemcpyDeviceToDevice);
  // }
}

int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 1024;
  constexpr auto n_threads = 256;  // defined in the shader

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

  // PushConstants pc = {n_input, 0};
  // algorithm->set_push_constants(pc);

  // seq->record_commands_with_blocks(algorithm.get(), n_blocks);
  // seq->launch_kernel_async();
  // seq->sync();

  gpu_merge_sort(algorithm.get(),
                 seq.get(),
                 1,
                 u_input_buf.get(),
                 u_output_buf.get(),
                 n_input);

  // print out the

  for (int i = 0; i < n_input; i++) {
    spdlog::info("{}", u_input_buf->at(i));
  }

  bool is_sorted = std::is_sorted(u_input_buf->begin(), u_input_buf->end());
  spdlog::info("is_sorted: {}", is_sorted);

  bool is_sorted_alt = std::is_sorted(u_output_buf->begin(), u_output_buf->end());
  spdlog::info("is_sorted_alt: {}", is_sorted_alt);

  return 0;
}