#pragma once

#include <cstddef>

#include "base_engine.hpp"
#include "vk_mem_alloc.h"

class Buffer {
 public:
  Buffer() = delete;
  explicit Buffer(VkDevice device,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                  VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
                  VmaAllocationCreateFlags flags =
                      VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT);

  ~Buffer();

  void destroy();

  // ---------------------------------------------------------------------------
  //      The following functions provides infos for the descriptor set
  // ---------------------------------------------------------------------------

  [[nodiscard]] VkDescriptorBufferInfo construct_descriptor_buffer_info() const;

 private:
  VkDevice device_ = VK_NULL_HANDLE;

  VkBuffer buffer_ = VK_NULL_HANDLE;

  // Vulkan Memory Allocator components
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkDeviceSize size_ = 0;

  // Raw pointer to the mapped data, CPU/GPU shared memory.
  std::byte *mapped_data_ = nullptr;

  bool persistent_ = true;
};
