#include "sequence.hpp"

#include <spdlog/spdlog.h>

#include "vk_helper.hpp"

void Sequence::destroy() {
  spdlog::debug("Sequence::destroy()");

  vkFreeCommandBuffers(device_, command_pool_, 1, &command_buffer_);
  vkDestroyCommandPool(device_, command_pool_, nullptr);
  vkDestroyFence(device_, fence_, nullptr);
}

void Sequence::create_command_pool() {
  spdlog::debug("Sequence::create_command_pool()");

  const VkCommandPoolCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      .queueFamilyIndex = compute_queue_index_,
  };

  check_vk_result(
      vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_));
}

void Sequence::create_command_buffer() {
  spdlog::debug("Sequence::create_command_buffer()");

  const VkCommandBufferAllocateInfo alloc_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = command_pool_,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1,
  };

  check_vk_result(
      vkAllocateCommandBuffers(device_, &alloc_info, &command_buffer_));
}

void Sequence::create_sync_objects() {
  spdlog::debug("Sequence::create_sync_objects()");

  constexpr VkFenceCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
  };

  check_vk_result(vkCreateFence(device_, &create_info, nullptr, &fence_));
}
