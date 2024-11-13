#pragma once

#include <cstring>
#include <span>

#include "base_engine.hpp"
#include "vk_mem_alloc.h"
#include "vulkan_resource.hpp"

class Buffer : public VulkanResource<VkBuffer> {
 public:
  Buffer() = delete;
  explicit Buffer(std::shared_ptr<VkDevice> device_ptr,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage =
                      VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                  VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
                  VmaAllocationCreateFlags flags =
                      VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT);

  ~Buffer() override { destroy(); }

 protected:
  void destroy() override;

 public:
  // ---------------------------------------------------------------------------
  //      The following functions provides infos for the descriptor set
  // ---------------------------------------------------------------------------

  [[nodiscard]] VkDescriptorBufferInfo construct_descriptor_buffer_info() const;

  template <typename T>
  [[nodiscard]] T* map() {
    return reinterpret_cast<T*>(mapped_data_);
  }

  template <typename T>
  std::span<T> span() {
    return std::span<T>(map<T>(), size_ / sizeof(T));
  }

  // fill the buffer with the given value
  template <typename T>
  void fill(const T& value) {
    // std::fill(map<T>(), map<T>() + size_ / sizeof(T), value);
    std::fill_n(map<T>(), size_ / sizeof(T), value);
  }

  void zeros() { fill(0); }
  void ones() { fill(1); }

  // ---------------------------------------------------------------------------
  // getters
  // ---------------------------------------------------------------------------

  [[nodiscard]] VkDeviceSize get_size() const { return size_; }

 private:
  // Vulkan Memory Allocator components
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  VkDeviceSize size_ = 0;

  // Raw pointer to the mapped data, CPU/GPU shared memory.
  std::byte* mapped_data_ = nullptr;

  bool persistent_ = true;

  friend class Engine;
};
