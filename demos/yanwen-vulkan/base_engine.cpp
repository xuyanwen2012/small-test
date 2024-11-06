#include "base_engine.hpp"

#define VK_NO_PROTOTYPES
#include "volk.h"

#include <stdexcept>
#include <cstring>

BaseEngine::BaseEngine(bool enable_validation) : enable_validation_(enable_validation) {
  spdlog::debug("BaseEngine::BaseEngine");

  try {
    device_initialization();
  } catch (std::exception& e) {
    spdlog::error(e.what());
    throw;
  }
}

void BaseEngine::destroy() {
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
}

bool BaseEngine::check_validation_layer_support(const char* layer_name) {
  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const auto& layer_properties : available_layers) {
    if (strcmp(layer_name, layer_properties.layerName) == 0) {
      return true;
    }
  }
  return false;
}

std::vector<const char*> BaseEngine::get_required_extensions() {
  std::vector<const char*> extensions;
  
  if (enable_validation_) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }
  
  return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL BaseEngine::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
    void* p_user_data) {
  if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    spdlog::error("Validation layer: {}", p_callback_data->pMessage);
  } else {
    spdlog::debug("Validation layer: {}", p_callback_data->pMessage);
  }
  return VK_FALSE;
}

void BaseEngine::device_initialization() {
  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    throw std::runtime_error("Failed to initialize Volk");
  }

  // Create instance
  VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Hello Vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3
  };

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
  };

  // Setup validation layers
  constexpr const char* validation_layer = "VK_LAYER_KHRONOS_validation";
  std::vector<const char*> validation_layers = {validation_layer};

  auto extensions = get_required_extensions();
  create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  create_info.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
  if (enable_validation_) {
    if (check_validation_layer_support(validation_layer)) {
      create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
      create_info.ppEnabledLayerNames = validation_layers.data();

      debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
      debug_create_info.messageSeverity = 
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      debug_create_info.messageType =
          VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info.pfnUserCallback = debug_callback;

      create_info.pNext = &debug_create_info;
      spdlog::info("Validation layer enabled");
    } else {
      spdlog::warn("Validation layer requested but not available!");
    }
  }

  if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan instance");
  }

  volkLoadInstance(instance);

  // Select physical device
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
  if (device_count == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

  physical_device = VK_NULL_HANDLE;
  for (const auto& device : devices) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    vkGetPhysicalDeviceFeatures(device, &device_features);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

    bool has_compute_queue = false;
    for (const auto& queue_family : queue_families) {
      if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        has_compute_queue = true;
        break;
      }
    }

    if (device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && has_compute_queue) {
      physical_device = device;
      spdlog::info("Selected GPU: {}", device_properties.deviceName);
      break;
    }
  }

  if (physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error("Failed to find a suitable GPU");
  }

  // Create logical device
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

  compute_queue_family_index = UINT32_MAX;
  for (uint32_t i = 0; i < queue_family_count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      compute_queue_family_index = i;
      break;
    }
  }

  if (compute_queue_family_index == UINT32_MAX) {
    throw std::runtime_error("Could not find a compute queue family");
  }

  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = compute_queue_family_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority
  };

  VkPhysicalDeviceFeatures device_features{};

  VkDeviceCreateInfo device_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
      .enabledLayerCount = 0,
      .enabledExtensionCount = 0,
      .pEnabledFeatures = &device_features
  };

  if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  vkGetDeviceQueue(device, compute_queue_family_index, 0, &compute_queue);
}
