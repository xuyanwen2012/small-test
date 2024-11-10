#include "base_engine.hpp"

#include <spdlog/spdlog.h>

#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

VmaAllocator BaseEngine::vma_allocator;

void BaseEngine::destroy() {
  spdlog::debug("BaseEngine::destroy");

  vmaDestroyAllocator(vma_allocator);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void BaseEngine::initialize_device() {
  spdlog::debug("BaseEngine::initialize_device");

  if (volkInitialize() != VK_SUCCESS) {
    spdlog::error("Failed to initialize Volk");
    return;
  }

  // Add validation layer check
  constexpr const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";
  bool validation_layers_available = false;

  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
  std::vector<VkLayerProperties> available_layers(layer_count);
  vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

  for (const auto& layer : available_layers) {
    if (strcmp(validation_layer_name, layer.layerName) == 0) {
      validation_layers_available = true;
      break;
    }
  }

  if (validation_layers_available) {
    spdlog::info("Validation layer {} is available", validation_layer_name);
  } else {
    spdlog::warn("Validation layer {} not available, continuing without it",
                 validation_layer_name);
  }

  // Create Vulkan instance
  constexpr VkApplicationInfo app_info{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Minimal Vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  // Modify instance creation info to include validation layer if available
  const char* enabled_layers[] = {validation_layer_name};

  VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
      .enabledLayerCount = validation_layers_available ? 1u : 0u,
      .ppEnabledLayerNames =
          validation_layers_available ? enabled_layers : nullptr,
  };

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    spdlog::error("Failed to create Vulkan instance");
    return;
  }

  // Load instance functions
  volkLoadInstance(instance_);

  // Enumerate physical devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  if (deviceCount == 0) {
    spdlog::error("Failed to find GPUs with Vulkan support");
    return;
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, physicalDevices.data());

  // Select the first physical device for convenience
  physical_device_ = physicalDevices[0];

  // check if this physical device is integrated GPU with compute capability
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physical_device_, &deviceProperties);
  if (deviceProperties.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
    spdlog::warn("This physical device is not an integrated GPU");
    return;
  }

  // print the name of the physical device
  spdlog::info("Physical device: {}", deviceProperties.deviceName);

  // Create a logical device
  constexpr float queue_priority = 1.0f;
  const VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = 0,  // assume
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  };

  const VkDeviceCreateInfo device_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queue_create_info,
  };

  if (vkCreateDevice(
          physical_device_, &device_create_info, nullptr, &device_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create logical device");
    return;
  }

  volkLoadDevice(device_);

  spdlog::info("Vulkan instance and device created successfully!");
}

void BaseEngine::vma_initialization() {
  const VmaVulkanFunctions vulkan_functions{
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
  };

  const VmaAllocatorCreateInfo vma_allocatorcreate_info{
      .physicalDevice = physical_device_,
      .device = device_,
      .pVulkanFunctions = &vulkan_functions,
      .instance = instance_,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };

  if (vmaCreateAllocator(&vma_allocatorcreate_info, &vma_allocator) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create VMA allocator");
    return;
  }

  spdlog::info("VMA allocator created successfully!");
}
