#pragma once

#define VK_NO_PROTOTYPES
#include <volk.h>

#include <memory>

class BaseEngine {
 public:
  BaseEngine() { initialize_device(); }
  ~BaseEngine() { destroy(); }

  void destroy();

  // ---------------------------------------------------------------------------
  //                  Getter and Setter
  // ---------------------------------------------------------------------------
  [[nodiscard]] std::shared_ptr<VkDevice> get_device_ptr() const {
    return std::make_shared<VkDevice>(device_);
  }

  [[nodiscard]] VkDevice& get_device() { return device_; }
  [[nodiscard]] const VkDevice& get_device() const { return device_; }

  [[nodiscard]] VkQueue& get_queue() { return queue_; }
  [[nodiscard]] const VkQueue& get_queue() const { return queue_; }

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
};