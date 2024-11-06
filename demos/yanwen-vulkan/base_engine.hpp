#pragma once

#define VK_NO_PROTOTYPES
#include "volk.h"

class BaseEngine {
 public:
  BaseEngine(bool enable_validation = false);
  ~BaseEngine() { destroy(); }

  void destroy();

 private:
  void initialize_device();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  bool enable_validation_;
};
