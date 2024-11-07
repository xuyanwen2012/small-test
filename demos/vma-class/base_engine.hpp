#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>

class BaseEngine {
 public:
  BaseEngine() {
    initialize_device();
    vma_initialization();
  }
  ~BaseEngine() { destroy(); }

  void destroy();

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
};