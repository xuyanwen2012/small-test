#include "vulkan/base_engine.hpp"

#include <spdlog/spdlog.h>

#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

VmaAllocator BaseEngine::vma_allocator;

void BaseEngine::destroy() {
  spdlog::debug("BaseEngine::destroy()");

  vmaDestroyAllocator(vma_allocator);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void BaseEngine::initialize_device() {
  spdlog::debug("BaseEngine::initialize_device()");

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

  // Add this before device creation
  uint32_t queue_family_count = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_, &queue_family_count, nullptr);
  std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(
      physical_device_, &queue_family_count, queue_families.data());

  // Find queue family with compute support
  bool found = false;
  for (uint32_t i = 0; i < queue_family_count; i++) {
    if (queue_families[i].queueFlags & VK_QUEUE_COMPUTE_BIT) {
      compute_queue_index_ = i;
      found = true;
      break;
    }
  }

  if (!found) {
    throw std::runtime_error(
        "Could not find a queue family that supports compute operations");
  }

  // Create a logical device (compute queue)
  constexpr float queue_priority = 1.0f;
  const VkDeviceQueueCreateInfo queue_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = compute_queue_index_,
      .queueCount = 1,
      .pQueuePriorities = &queue_priority,
  };

  // need these features for 8-bit integer operations for some kernels (e.g.,
  // radix tree)
  constexpr VkPhysicalDeviceVulkan12Features vulkan12Features{
      .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .storageBuffer8BitAccess = VK_TRUE,
      .shaderInt8 = VK_TRUE,
      .bufferDeviceAddress = VK_TRUE,
  };

  // Modify the device creation info to include the features
  const VkDeviceCreateInfo device_create_info{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = &vulkan12Features,
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

  vkGetDeviceQueue(device_, compute_queue_index_, 0, &queue_);

  spdlog::debug("Vulkan instance and device created successfully!");

  // print some information about what we created
  spdlog::debug("\tQueue family index: {}", compute_queue_index_);
  spdlog::debug("\tQueue: {}", (uint64_t)queue_);
}

void BaseEngine::vma_initialization() {
  const VmaVulkanFunctions vulkan_functions{
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
  };

  const VmaAllocatorCreateInfo vma_allocator_create_info{
      .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
      .physicalDevice = physical_device_,
      .device = device_,
      .pVulkanFunctions = &vulkan_functions,
      .instance = instance_,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };

  if (vmaCreateAllocator(&vma_allocator_create_info, &vma_allocator) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create VMA allocator");
    return;
  }

  spdlog::debug("VMA allocator created successfully!");
}
