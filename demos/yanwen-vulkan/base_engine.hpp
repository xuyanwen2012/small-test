#pragma once

#include <spdlog/spdlog.h>

#define VK_NO_PROTOTYPES
#include <vector>

#include "volk.h"

class BaseEngine {
 public:
  BaseEngine(bool enable_validation = false);
  ~BaseEngine() {
    spdlog::debug("BaseEngine::~BaseEngine");
    destroy();
  }

  void destroy();

 private:
  void device_initialization();
  void get_queues();
  void vma_initialization() const;
  bool check_validation_layer_support(const char* layer_name);
  std::vector<const char*> get_required_extensions();
  
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                 VkDebugUtilsMessageTypeFlagsEXT message_type,
                 const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
                 void* p_user_data);

 protected:
  VkInstance instance;
  VkPhysicalDevice physical_device;
  VkDevice device;
  VkQueue compute_queue;
  uint32_t compute_queue_family_index;
  bool enable_validation_;
};
