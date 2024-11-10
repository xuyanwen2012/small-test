#include "sequence.hpp"

#include <spdlog/spdlog.h>

#include "vk_helper.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

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

void Sequence::cmd_begin() const {
  spdlog::debug("Sequence::cmd_begin()");

  const VkCommandBufferBeginInfo begin_info = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  check_vk_result(vkBeginCommandBuffer(command_buffer_, &begin_info));
}

void Sequence::cmd_end() const {
  spdlog::debug("Sequence::cmd_end()");

  check_vk_result(vkEndCommandBuffer(command_buffer_));
}

void Sequence::launch_kernel_async() {
  spdlog::debug("Sequence::launch_kernel_async()");

  const VkSubmitInfo submit_info = {
      .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = 1,
      .pCommandBuffers = &command_buffer_,
  };

  check_vk_result(vkQueueSubmit(queue_, 1, &submit_info, fence_));
}

void Sequence::sync() const {
  spdlog::debug("Sequence::sync()");

  check_vk_result(vkWaitForFences(device_, 1, &fence_, VK_TRUE, UINT64_MAX));
  check_vk_result(vkResetFences(device_, 1, &fence_));
}
