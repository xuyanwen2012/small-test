#pragma once

#define VK_NO_PROTOTYPES 1
#include "volk.h"

#include "vk_mem_alloc.h"

class BaseEngine {
 public:
  BaseEngine(bool enable_validation = false);
  ~BaseEngine() { destroy(); }

  void destroy();

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;

  // Vulkan Memory Allocator
  VmaAllocator allocator_ = VK_NULL_HANDLE;

  bool enable_validation_;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
};
