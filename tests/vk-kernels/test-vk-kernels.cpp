#include <gtest/gtest.h>
#include <volk.h>

#include <glm/glm.hpp>

#include "vulkan/engine.hpp"

constexpr int small_n_points = 1024;
constexpr int medium_n_points = 640 * 48;    // ~30k points
constexpr int large_n_points = 1024 * 1024;  // 1M points

constexpr float min_coord = 0.0f;
constexpr float range = 1024.0f;
constexpr int seed = 114514;

class VulkanKernelsTest : public ::testing::Test {
 protected:
  Engine engine;

  void ValidatePoints(const std::span<glm::vec4>& points,
                      float min_val,
                      float range) {
    for (const auto& point : points) {
      // Check X coordinate
      EXPECT_GE(point.x, min_val) << "X coordinate below minimum";
      EXPECT_LT(point.x, min_val + range) << "X coordinate above range";

      // Check Y coordinate
      EXPECT_GE(point.y, min_val) << "Y coordinate below minimum";
      EXPECT_LT(point.y, min_val + range) << "Y coordinate above range";

      // Check Z coordinate
      EXPECT_GE(point.z, min_val) << "Z coordinate below minimum";
      EXPECT_LT(point.z, min_val + range) << "Z coordinate above range";

      // Check W coordinate
      EXPECT_GE(point.w, min_val) << "W coordinate below minimum";
      EXPECT_LT(point.w, min_val + range) << "W coordinate above range";
    }
  }

  void RunInitTest(int n_points);
};

TEST_F(VulkanKernelsTest, InitSmall) { RunInitTest(small_n_points); }

TEST_F(VulkanKernelsTest, InitMedium) { RunInitTest(medium_n_points); }

TEST_F(VulkanKernelsTest, InitLarge) { RunInitTest(large_n_points); }

void VulkanKernelsTest::RunInitTest(int n_points) {
  struct {
    int n;
    int min_val;
    int range;
    int seed;
  } init_push_constants = {
      n_points, static_cast<int>(min_coord), static_cast<int>(range), seed};

  auto seq = engine.sequence();
  auto u_points = engine.buffer(n_points * sizeof(glm::vec4));

  auto algo =
      engine.algorithm("init.spv",
                       {u_points},
                       512,
                       reinterpret_cast<const std::byte*>(&init_push_constants),
                       sizeof(init_push_constants));

  // Calculate optimal number of blocks based on size
  const auto num_blocks =
      (n_points + 511) / 512;  // Ceiling division by workgroup size

  seq->record_commands_with_blocks(algo.get(), num_blocks);
  seq->launch_kernel_async();
  seq->sync();

  // Validate results
  auto points_span = u_points->span<glm::vec4>();
  ValidatePoints(points_span, min_coord, range);
}

int main(int argc, char** argv) {
  spdlog::set_level(spdlog::level::off);
  //   spdlog::set_level(spdlog::level::debug);

  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
