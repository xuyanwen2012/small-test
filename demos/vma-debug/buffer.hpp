#pragma once

// #include <cstddef>

#include <span>

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

  template <typename T>
  [[nodiscard]] T* map() {
    return reinterpret_cast<T*>(mapped_data_);
  }

  // write helper function using std::span to easily read/write to the buffer.
  // Can we not use memcpy??
  template <typename T>
  std::span<T> span() {
    return std::span<T>(map<T>(), size_ / sizeof(T));
  }

 private:
  VkDevice device_ = VK_NULL_HANDLE;

  VkBuffer buffer_ = VK_NULL_HANDLE;

  // Vulkan Memory Allocator components
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkDeviceSize size_ = 0;

  // Raw pointer to the mapped data, CPU/GPU shared memory.
  std::byte* mapped_data_ = nullptr;

  bool persistent_ = true;
};
