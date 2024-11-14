#include "test-base.hpp"

class VulkanMergeSortKernelsParamTest
    : public VulkanKernelTestBase,
      public ::testing::WithParamInterface<InitTestParams> {
 protected:
  void RunMergeSortTestWithBlocks(int n_points, int num_blocks);
};

// Test implementation
TEST_P(VulkanMergeSortKernelsParamTest, MergeSortTest) {
  const auto& params = GetParam();
  RunMergeSortTestWithBlocks(params.n_points, params.n_blocks);
}

// Test cases
INSTANTIATE_TEST_SUITE_P(
    MergeSortSweep,
    VulkanMergeSortKernelsParamTest,
    ::testing::Values(
        // Small input tests
        InitTestParams{1024, 1, "Small_1Block"},
        InitTestParams{1024, 2, "Small_2Blocks"},
        InitTestParams{1024, 3, "Small_3Blocks"},
        InitTestParams{1024, 4, "Small_4Blocks"},
        // Medium input tests
        InitTestParams{640 * 48, 1, "Medium_1Block"},
        InitTestParams{640 * 48, 2, "Medium_2Blocks"},
        InitTestParams{640 * 48, 3, "Medium_3Blocks"},
        InitTestParams{640 * 48, 4, "Medium_4Blocks"},
        // Large input tests
        InitTestParams{1024 * 1024, 1, "Large_1Block"},
        InitTestParams{1024 * 1024, 2, "Large_2Blocks"},
        InitTestParams{1024 * 1024, 3, "Large_3Blocks"},
        InitTestParams{1024 * 1024, 4, "Large_4Blocks"},
        // Irregular input size (not a multiple of 512  )
        InitTestParams{12345, 1, "Irregular_1Block"},
        InitTestParams{99999, 2, "Irregular_2Blocks"},
        InitTestParams{1145142, 3, "Irregular_3Blocks"},
        InitTestParams{124, 4, "Irregular_4Blocks"}),
    [](const testing::TestParamInfo<InitTestParams>& info) {
      return info.param.name;
    });

void VulkanMergeSortKernelsParamTest::RunMergeSortTestWithBlocks(
    int n_points, int n_physical_blocks) {
  struct PushConstants {
    uint32_t n_logical_blocks;
    uint32_t n;
    uint32_t width;
    uint32_t num_pairs;
  };

  // ---------------------------------------------------------------------------
  // Prepare the buffers
  // ---------------------------------------------------------------------------

  auto u_input_buf = engine.typed_buffer<uint32_t>(n_points);
  u_input_buf->random(1, n_points);

  auto u_output_buf = engine.typed_buffer<uint32_t>(n_points);
  u_output_buf->zeros();

  // ---------------------------------------------------------------------------
  // Run the kernel
  // ---------------------------------------------------------------------------

  auto algorithm = engine.algorithm(
      "merge_sort.spv", {u_input_buf, u_output_buf}, sizeof(PushConstants));
  auto seq = engine.sequence();

  // ---------------------------------------------------------------------------
  constexpr auto threads_per_block = 256;

  for (int width = 1; width < n_points; width *= 2) {
    int num_pairs = (n_points + 2 * width - 1) / (2 * width);
    int total_threads = num_pairs;
    int logical_blocks =
        (total_threads + threads_per_block - 1) / threads_per_block;

    const PushConstants pc = {uint32_t(logical_blocks),
                              uint32_t(n_points),
                              uint32_t(width),
                              uint32_t(num_pairs)};
    algorithm->set_push_constants(pc);
    // should already be swapped by the previous iteration
    algorithm->update_descriptor_sets_with_buffers({u_input_buf, u_output_buf});

    seq->record_commands_with_blocks(algorithm.get(), n_physical_blocks);
    seq->launch_kernel_async();
    seq->sync();

    std::swap(u_input_buf, u_output_buf);
  }

  // ---------------------------------------------------------------------------
  // Validate the results
  // ---------------------------------------------------------------------------

  auto buf_a_sorted = std::ranges::is_sorted(*u_input_buf);
  auto buf_b_sorted = std::ranges::is_sorted(*u_output_buf);

  EXPECT_TRUE(buf_a_sorted);
  EXPECT_FALSE(buf_b_sorted);
  EXPECT_NE(buf_a_sorted, buf_b_sorted);
}
