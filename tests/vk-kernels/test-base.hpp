#pragma once

#include <gtest/gtest.h>
#include <volk.h>
#include <glm/glm.hpp>

#include "common.hpp"
#include "vulkan/engine.hpp"

// Base test fixture class that contains common functionality
class VulkanKernelTestBase : public ::testing::Test {
 protected:
  Engine engine;

  void ValidatePoints(const std::span<glm::vec4>& points,
                     float min_val,
                     float range) {
    int uninitialized_count = 0;

    for (size_t i = 0; i < points.size(); i++) {
      const auto& point = points[i];

      // Check for uninitialized values
      if (point.x == 0.0f && point.y == 0.0f && point.z == 0.0f &&
          point.w == 0.0f) {
        uninitialized_count++;
        FAIL() << "Found uninitialized value at index " << i;
      }

      // Range checks
      EXPECT_GE(point.x, min_val) << "X coordinate below minimum at index " << i;
      EXPECT_LT(point.x, min_val + range) << "X coordinate above range at index " << i;
      // ... similar checks for y, z, w ...
    }

    EXPECT_EQ(uninitialized_count, 0)
        << "Found " << uninitialized_count << " uninitialized values";
  }
}; 