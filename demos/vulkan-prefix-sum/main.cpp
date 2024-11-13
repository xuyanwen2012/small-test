#include <spdlog/spdlog.h>

#include <numeric>

#include "vulkan/engine.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 1234;
  constexpr auto n_threads = 256;  // defined in the shader
  constexpr auto n_blocks = (n_input + n_threads - 1) / n_threads;

  // constexpr auto n_blocks = 2;

  auto input = engine.buffer(n_input * sizeof(uint32_t));
  input->fill(1);

  auto output = engine.buffer(n_input * sizeof(uint32_t));
  output->zeros();

  auto block_sums = engine.buffer(n_blocks * sizeof(uint32_t));
  block_sums->zeros();

  struct {
    uint n;
    uint block_size;  // Must match workgroup size
  } pc = {n_input, n_threads};

  auto algo = engine.algorithm("naive_prefix_sum.spv",
                               {
                                   input,
                                   output,
                                   block_sums,
                               },
                               n_threads,
                               reinterpret_cast<const std::byte*>(&pc),
                               sizeof(pc));

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algo.get(), n_blocks);
  seq->launch_kernel_async();
  seq->sync();

  auto algo2 = engine.algorithm("block_add.spv",
                                {
                                    block_sums,
                                    output,
                                },
                                n_threads,
                                reinterpret_cast<const std::byte*>(&pc),
                                sizeof(pc));

  auto seq2 = engine.sequence();
  seq2->record_commands_with_blocks(algo2.get(), n_blocks);
  seq2->launch_kernel_async();
  seq2->sync();

  // expected result

  // auto expected = input->span<uint32_t>();
  // make a copy of the input
  std::vector<uint32_t> cpu_input(n_input);
  std::ranges::copy(input->span<uint32_t>(), cpu_input.begin());

  std::vector<uint32_t> cpu_expected(n_input);
  std::partial_sum(cpu_input.begin(), cpu_input.end(), cpu_expected.begin());

  // print all
  for (auto i = 0; i < n_input; ++i) {
    spdlog::info("[{}] {} {}", i, output->span<uint32_t>()[i], cpu_expected[i]);
  }

  bool is_equal = std::ranges::equal(output->span<uint32_t>(), cpu_expected);
  spdlog::info("Result: {}", is_equal);

  return 0;
}