#include <glm/glm.hpp>
#include <random>

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
                      InitTestParams{640 * 480, 1, "Medium_1Block"},
                      InitTestParams{640 * 480, 2, "Medium_2Blocks"},
                      InitTestParams{1920 * 1080, 1, "Large_1Block"},
                      InitTestParams{1920 * 1080, 2, "Large_2Blocks"}));

void VulkanMortonKernelTest::RunMortonTestWithBlocks(int n_points,
                                                     int num_blocks) {
  auto u_points = engine.buffer(n_points * sizeof(glm::vec4));
  auto u_morton_keys = engine.buffer(n_points * sizeof(uint));

  // prepare random points
  {
    std::random_device rd;
    std::mt19937 gen(seed);
    std::uniform_real_distribution<> dis(min_coord, min_coord + range);

    for (int i = 0; i < n_points; ++i) {
      u_points->span<glm::vec4>()[i] =
          glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
    }
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

  std::ranges::sort(morton_keys);

  // check if the morton keys are sorted
  for (size_t i = 1; i < morton_keys.size(); ++i) {
    EXPECT_LE(morton_keys[i - 1], morton_keys[i]);
  }

  // check not zeros
  for (int i = 0; i < n_points; ++i) {
    EXPECT_NE(morton_keys[i], 0);
  }
}
