#include <gtest/gtest.h>
#include <volk.h>

#include <glm/glm.hpp>

#include "vulkan/engine.hpp"

constexpr float min_coord = 0.0f;
constexpr float range = 1024.0f;
constexpr int seed = 114514;

// Test parameters struct
struct InitTestParams {
  int n_points;
  int n_blocks;
  std::string name;  // For readable test output

  // For pretty-printing in test output
  friend std::ostream& operator<<(std::ostream& os,
                                  const InitTestParams& params) {
    return os << params.name;
  }
};

class VulkanKernelsParamTest : public ::testing::TestWithParam<InitTestParams> {
 protected:
  Engine engine;

  void ValidatePoints(const std::span<glm::vec4>& points,
                      float min_val,
                      float range) {
    int uninitialized_count = 0;

    for (size_t i = 0; i < points.size(); i++) {
      const auto& point = points[i];

      // Check for uninitialized values (exactly 0 in all components would be
      // extremely unlikely)
      if (point.x == 0.0f && point.y == 0.0f && point.z == 0.0f &&
          point.w == 0.0f) {
        uninitialized_count++;
        FAIL() << "Found uninitialized value at index " << i
               << ". This suggests the kernel didn't process all elements.";
      }

      // Existing range checks
      EXPECT_GE(point.x, min_val)
          << "X coordinate below minimum at index " << i;
      EXPECT_LT(point.x, min_val + range)
          << "X coordinate above range at index " << i;

      EXPECT_GE(point.y, min_val)
          << "Y coordinate below minimum at index " << i;
      EXPECT_LT(point.y, min_val + range)
          << "Y coordinate above range at index " << i;

      EXPECT_GE(point.z, min_val)
          << "Z coordinate below minimum at index " << i;
      EXPECT_LT(point.z, min_val + range)
          << "Z coordinate above range at index " << i;

      EXPECT_GE(point.w, min_val)
          << "W coordinate below minimum at index " << i;
      EXPECT_LT(point.w, min_val + range)
          << "W coordinate above range at index " << i;
    }

    EXPECT_EQ(uninitialized_count, 0)
        << "Found " << uninitialized_count << " uninitialized values out of "
        << points.size() << " total points";
  }

  void RunInitTestWithBlocks(int n_points, int num_blocks);
};

// Define test cases
TEST_P(VulkanKernelsParamTest, InitTest) {
  const auto& params = GetParam();
  RunInitTestWithBlocks(params.n_points, params.n_blocks);
}

// Generate test cases
INSTANTIATE_TEST_SUITE_P(
    InitSweep,
    VulkanKernelsParamTest,
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
      return info.param.name;  // Use the name field from our struct
    });

void VulkanKernelsParamTest::RunInitTestWithBlocks(int n_points,
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
      engine.algorithm("init.spv",
                       {u_points},
                       512,
                       reinterpret_cast<const std::byte*>(&init_push_constants),
                       sizeof(init_push_constants));

  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // Validate results
  auto points_span = u_points->span<glm::vec4>();
  ValidatePoints(points_span, min_coord, range);
}

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::off);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
