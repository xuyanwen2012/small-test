#pragma once

#include <cstring>
#include <span>
#include <vector>

#include "base_engine.hpp"
#include "vk_mem_alloc.h"
#include "vulkan_resource.hpp"

/**
 * @brief A wrapper class for Vulkan buffer objects that provides CPU/GPU shared
 * memory functionality
 *
 * This class manages a Vulkan buffer that can be accessed by both CPU and GPU.
 * It uses the Vulkan Memory Allocator (VMA) library to handle memory allocation
 * and provides convenient methods for reading from and writing to the buffer.
 */
class Buffer : public VulkanResource<VkBuffer> {
 public:
  Buffer() = delete;

  /**
   * @brief Creates a new buffer with CPU/GPU shared memory
   *
   * @param device_ptr    Shared pointer to the Vulkan logical device
   * @param size         Size of the buffer in bytes
   * @param usage        Vulkan buffer usage flags. Defaults to
   * VK_BUFFER_USAGE_STORAGE_BUFFER_BIT which allows the buffer to be used as a
   * storage buffer in shaders
   * @param memory_usage VMA memory usage hint. Defaults to
   * VMA_MEMORY_USAGE_AUTO which lets VMA choose the best memory type based on
   * the usage patterns
   * @param flags        VMA allocation flags. Defaults to:
   *                     - VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT: Enables
   * random access from CPU
   *                     - VMA_ALLOCATION_CREATE_MAPPED_BIT: Keeps the memory
   * persistently mapped These flags together ensure the buffer is suitable for
   * CPU/GPU shared memory with efficient CPU access
   */
  explicit Buffer(std::shared_ptr<VkDevice> device_ptr,
                  VkDeviceSize size,
                  VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                  VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO,
                  VmaAllocationCreateFlags flags =
                      VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                      VMA_ALLOCATION_CREATE_MAPPED_BIT);

  ~Buffer() override { destroy(); }

 protected:
  void destroy() override;

 public:
  // The following functions provides infos for the descriptor set
  [[nodiscard]] VkDescriptorBufferInfo construct_descriptor_buffer_info() const;

  // ---------------------------------------------------------------------------
  // Helper functions for accessing the buffer data
  // ---------------------------------------------------------------------------
  template <typename T>
  [[deprecated("Use as() instead.")]] [[nodiscard]] T* map() {
    return reinterpret_cast<T*>(mapped_data_);
  }

  template <typename T>
  [[nodiscard]] T* as() {
    return reinterpret_cast<T*>(mapped_data_);
  }

  // most convenient way to access the buffer data, we can use
  // std::ranges::for_each etc.
  template <typename T>
  std::span<T> span() {
    return std::span<T>(as<T>(), size_ / sizeof(T));
  }

  // fill the buffer with the given value
  template <typename T>
  void fill(const T& value) {
    std::fill_n(as<T>(), size_ / sizeof(T), value);
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

  friend class Engine;
};
