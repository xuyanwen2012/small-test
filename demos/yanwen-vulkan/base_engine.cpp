#include "base_engine.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>
#include <vector>

BaseEngine::BaseEngine(bool enable_validation)
    : enable_validation_(enable_validation) {
  spdlog::debug("BaseEngine::BaseEngine");

  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    throw std::runtime_error("Failed to initialize Volk");
  }

  initialize_device();
}

void BaseEngine::destroy() {
  spdlog::debug("BaseEngine::destroy");

  if (device_ != VK_NULL_HANDLE) {
    vkDestroyDevice(device_, nullptr);
    device_ = VK_NULL_HANDLE;
  }
  if (instance_ != VK_NULL_HANDLE) {
    vkDestroyInstance(instance_, nullptr);
    instance_ = VK_NULL_HANDLE;
  }
}

void BaseEngine::initialize_device() {
  // Create instance
  VkApplicationInfo app_info{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .pApplicationName = "Vulkan App",
                             .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                             .apiVersion = VK_API_VERSION_1_3};

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
  };

  // Setup validation if requested
  std::vector<const char*> validation_layers;
  if (enable_validation_) {
    // Check validation layer availability
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
    std::vector<VkLayerProperties> available_layers(layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

    bool layer_found = false;
    for (const auto& layer : available_layers) {
      if (strcmp("VK_LAYER_KHRONOS_validation", layer.layerName) == 0) {
        validation_layers.push_back("VK_LAYER_KHRONOS_validation");
        layer_found = true;
        break;
      }
    }

    if (layer_found) {
      create_info.enabledLayerCount =
          static_cast<uint32_t>(validation_layers.size());
      create_info.ppEnabledLayerNames = validation_layers.data();
      spdlog::info("Validation layer enabled");
    } else {
      spdlog::warn("Validation layer requested but not available");
    }
  }

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create Vulkan instance");
  }

  // Load instance functions
  volkLoadInstance(instance_);

  // Select physical device
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices(instance_, &device_count, nullptr);
  if (device_count == 0) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support");
  }

  std::vector<VkPhysicalDevice> devices(device_count);
  vkEnumeratePhysicalDevices(instance_, &device_count, devices.data());

  // Find a suitable GPU with compute capabilities
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;
  for (const auto& device : devices) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);

    // Get queue family properties
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, nullptr);
    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queue_family_count, queue_families.data());

    // Check if device has compute queue
    bool has_compute = false;
    for (const auto& queue_family : queue_families) {
      if (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) {
        has_compute = true;
        break;
      }
    }

    // Select integrated GPU if it has compute capabilities
    if (device_properties.deviceType ==
            VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU &&
        has_compute) {
      physical_device = device;
      break;
    }
  }

  if (physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Failed to find suitable GPU with compute capabilities");
  }

  // Create logical device
  uint32_t queue_family_index =
      0;  // Assume first queue family supports compute
  float queue_priority = 1.0f;
  VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = queue_family_index,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority};

  VkDeviceCreateInfo device_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
      .enabledLayerCount = 0,
      .pEnabledFeatures = nullptr};

  if (vkCreateDevice(physical_device, &device_create_info, nullptr, &device_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create logical device");
  }

  vkGetDeviceQueue(device_, queue_family_index, 0, &queue_);
}
