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

  [[nodiscard]] VmaAllocator get_allocator() const { return allocator_; }

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;

  VmaAllocator allocator_ = VK_NULL_HANDLE;
};
