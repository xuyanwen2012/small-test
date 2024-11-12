#include <glm/glm.hpp>

#include "common.hpp"
#include "gtest/gtest.h"
#include "test-base.hpp"

class VulkanMortonKernelTest
    : public VulkanKernelTestBase,
      public ::testing::WithParamInterface<InitTestParams> {
 protected:
  void RunMortonTestWithBlocks(int n_points, int num_blocks);
};

TEST_P(VulkanMortonKernelTest, MortonTest) {
  const auto& params = GetParam();
  RunMortonTestWithBlocks(params.n_points, params.n_blocks);
}

INSTANTIATE_TEST_SUITE_P(
    MortonSweep,
    VulkanMortonKernelTest,
    ::testing::Values(InitTestParams{1024, 1, "Small_1Block"},
                      InitTestParams{1024, 2, "Small_2Blocks"},
                      InitTestParams{1024, 3, "Small_3Blocks"}));

void VulkanMortonKernelTest::RunMortonTestWithBlocks(int n_points,
                                                     int num_blocks) {
  auto u_points = engine.buffer(n_points * sizeof(glm::vec4));
  auto u_morton_keys = engine.buffer(n_points * sizeof(uint));

  //   u_points->fill(glm::vec4(0.0f));
  for (int i = 0; i < n_points; ++i) {
    u_points->span<glm::vec4>()[i] = glm::vec4(i, i, i, i);
  }

  u_morton_keys->zeros();

  struct PushConstants {
    uint n;
    float min_coord;
    float range;
  } pc = {static_cast<uint>(n_points), min_coord, range};

  auto algo = engine.algorithm("morton.spv",
                               {
                                   u_points,
                                   u_morton_keys,
                               },
                               768,
                               reinterpret_cast<const std::byte*>(&pc),
                               sizeof(pc));

  auto seq = engine.sequence();
  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  auto morton_keys = u_morton_keys->span<uint32_t>();

  // check if the morton keys are sorted
  for (size_t i = 1; i < morton_keys.size(); ++i) {
    EXPECT_LT(morton_keys[i - 1], morton_keys[i]);
  }
}
