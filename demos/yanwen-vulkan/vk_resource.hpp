// #pragma once

// #include <memory>

// #include "volk.h"

// template <typename HandleT>
// class VulkanResource {
//  public:
//   VulkanResource() = delete;
//   explicit VulkanResource(std::shared_ptr<VkDevice> device_ptr)
//       : device_ptr_(std::move(device_ptr)) {}

//   VulkanResource(const VulkanResource &) = delete;
//   VulkanResource(VulkanResource &&) = delete;

//   virtual ~VulkanResource() = default;

//   VulkanResource &operator=(const VulkanResource &) = delete;
//   VulkanResource &operator=(VulkanResource &&) = delete;

//   [[nodiscard]] HandleT &get_handle() { return handle_; }
//   [[nodiscard]] const HandleT &get_handle() const { return handle_; }

//  protected:
//   virtual void destroy() = 0;

//   std::shared_ptr<VkDevice> device_ptr_;
//   HandleT handle_;
// };