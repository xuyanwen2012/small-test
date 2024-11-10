#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>

#include "vk_mem_alloc.h"

// -----------------------------------------------------------------------------
// BaseEngine.hpp
// -----------------------------------------------------------------------------

class BaseEngine {
 public:
  BaseEngine() {
    initialize_device();
    vma_initialization();
  }
  ~BaseEngine() { destroy(); }

  void destroy();

  // getters
  [[nodiscard]] VkDevice get_device() const { return device_; }
  [[nodiscard]] VkQueue get_queue() const { return queue_; }
  [[nodiscard]] uint32_t get_compute_queue_index() const {
    return compute_queue_index_;
  }

  [[nodiscard]] static VmaAllocator get_allocator() { return vma_allocator; }

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;

  // device queue index for compute
  uint32_t compute_queue_index_ = 0;

  static VmaAllocator vma_allocator;
};
