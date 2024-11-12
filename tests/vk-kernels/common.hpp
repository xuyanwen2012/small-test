#pragma once

#include <glm/glm.hpp>
#include <span>
#include <string>

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
