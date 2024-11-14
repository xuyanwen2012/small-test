#include "test-base.hpp"

class VulkanInitKernelsParamTest
    : public VulkanKernelTestBase,
      public ::testing::WithParamInterface<InitTestParams> {
 protected:
  void RunInitTestWithBlocks(int n_points, int num_blocks);
};

// Test implementation
TEST_P(VulkanInitKernelsParamTest, InitTest) {
  const auto& params = GetParam();
  RunInitTestWithBlocks(params.n_points, params.n_blocks);
}

// Test cases
INSTANTIATE_TEST_SUITE_P(
    InitSweep,
    VulkanInitKernelsParamTest,
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
        InitTestParams{1, 3, "Irregular_3Blocks"},
        InitTestParams{124, 4, "Irregular_4Blocks"}),
    [](const testing::TestParamInfo<InitTestParams>& info) {
      return info.param.name;
    });

void VulkanInitKernelsParamTest::RunInitTestWithBlocks(int n_points,
                                                       int num_blocks) {
  struct {
    int n;
    float min_coord;
    float range;
    int seed;
  } init_push_constants = {n_points, min_coord, range, seed};

  auto seq = engine.sequence();
  auto u_points = engine.buffer(n_points * sizeof(glm::vec4));

  auto algo =
      engine.algorithm("init.spv", {u_points}, sizeof(init_push_constants));
  algo->set_push_constants(init_push_constants);

  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // Validate results
  auto points_span = u_points->span<glm::vec4>();
  ValidatePoints(points_span, min_coord, range);
}