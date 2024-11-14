#include <spdlog/spdlog.h>

#include <cstdint>
#include <numeric>

#include "vulkan/engine.hpp"

int main() {
  spdlog::set_level(spdlog::level::debug);

  Engine engine;

  constexpr auto n_input = 1234;
  constexpr auto n_threads = 256;  // defined in the shader
  constexpr auto n_blocks = (n_input + n_threads - 1) / n_threads;

  // ---------------------------------------------------------------------------
  // Prepare the data
  // ---------------------------------------------------------------------------

  auto input = engine.buffer(n_input * sizeof(uint32_t));
  input->ones();

  auto output = engine.buffer(n_input * sizeof(uint32_t));
  output->zeros();

  auto block_sums = engine.buffer(n_blocks * sizeof(uint32_t));
  block_sums->zeros();

  struct {
    uint n;
    uint block_size;  // Must match workgroup size
  } pc = {n_input, n_threads};

  // ---------------------------------------------------------------------------
  // Run the algorithm
  // ---------------------------------------------------------------------------

  auto algo = engine.algorithm("naive_prefix_sum.spv",
                               {
                                   input,
                                   output,
                                   block_sums,
                               },
                               sizeof(pc));
  algo->set_push_constants(pc);

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algo.get(), n_blocks);
  seq->launch_kernel_async();
  // seq->sync();

  auto algo2 = engine.algorithm("block_add.spv",
                                {
                                    block_sums,
                                    output,
                                },
                                sizeof(pc));
  algo2->set_push_constants(pc);

  auto seq2 = engine.sequence();
  seq2->record_commands_with_blocks(algo2.get(), n_blocks);
  seq2->launch_kernel_async();
  seq2->sync();

  // ---------------------------------------------------------------------------
  // Check the result
  // ---------------------------------------------------------------------------

  std::vector<uint32_t> cpu_input(n_input);
  std::ranges::copy(input->span<uint32_t>(), cpu_input.begin());

  std::vector<uint32_t> cpu_expected(n_input);
  std::partial_sum(cpu_input.begin(), cpu_input.end(), cpu_expected.begin());

  // print all
  for (auto i = 0; i < n_input; ++i) {
    spdlog::info("[{}] {} {}", i, output->span<uint32_t>()[i], cpu_expected[i]);
  }

  // print block sum
  for (auto s : block_sums->span<uint32_t>()) {
    spdlog::info("block sum: {}", s);
  }

  bool is_equal = std::ranges::equal(output->span<uint32_t>(), cpu_expected);
  spdlog::info("CPU and GPU result {} equal", is_equal ? "are" : "are not");

  return 0;
}