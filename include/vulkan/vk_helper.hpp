#pragma once

#include <volk.h>

#include <stdexcept>
#include <string>

[[nodiscard]] const char* VkResultToString(VkResult result);

constexpr void check_vk_result(const VkResult result) {
  if (result != VK_SUCCESS) {
    throw std::runtime_error("Vulkan error: " +
                             std::string(VkResultToString(result)));
  }
}
