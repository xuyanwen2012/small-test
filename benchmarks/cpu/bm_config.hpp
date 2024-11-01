#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <random>

#include "shared/structures.h"

namespace {

struct Config {
  static constexpr int DEFAULT_N = 640 * 480;  // ~300k
  static constexpr float DEFAULT_MIN_COORD = 0.0f;
  static constexpr float DEFAULT_RANGE = 1024.0f;
  static constexpr unsigned DEFAULT_SEED = 114514;
  static constexpr int DEFAULT_ITERATIONS = 40;
};

inline void gen_data(const std::shared_ptr<Pipe>& p, unsigned seed) {
  std::mt19937 gen(seed);
  std::uniform_real_distribution dis(
      Config::DEFAULT_MIN_COORD,
      Config::DEFAULT_MIN_COORD + Config::DEFAULT_RANGE);
  std::generate_n(p->u_points, Config::DEFAULT_N, [&dis, &gen] {
    return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
  });
}

}  // namespace
