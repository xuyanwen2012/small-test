#pragma once

#include <glm/glm.hpp>

template <>
struct fmt::formatter<glm::vec4> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }
  template <typename FormatContext>
  auto format(const glm::vec4& v, FormatContext& ctx) {
    return fmt::format_to(ctx.out(), "({}, {}, {}, {})", v.x, v.y, v.z, v.w);
  }
};

