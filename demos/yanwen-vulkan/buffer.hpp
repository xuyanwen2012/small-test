#pragma once

#include <spdlog/spdlog.h>

// #include "vk_resource.hpp"
#include "vma_usage.hpp"

class Buffer final {
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
