#include "base_engine.hpp"

#include <iostream>
#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

void BaseEngine::destroy() {
  vmaDestroyAllocator(allocator_);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void BaseEngine::initialize_device() {
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize Volk" << std::endl;
    return;
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

  const VkInstanceCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &app_info,
  };

  if (vkCreateInstance(&create_info, nullptr, &instance_) != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance" << std::endl;
    return;
  }

  // Load instance functions
  volkLoadInstance(instance_);

  // Enumerate physical devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  if (deviceCount == 0) {
    std::cerr << "Failed to find GPUs with Vulkan support" << std::endl;
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
    std::cerr << "This physical device is not an integrated GPU" << std::endl;
    return;
  }

  // print the name of the physical device
  std::cout << "Physical device: " << deviceProperties.deviceName << std::endl;

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
    std::cerr << "Failed to create logical device" << std::endl;
    return;
  }

  volkLoadDevice(device_);

  std::cout << "Vulkan instance and device created successfully!" << std::endl;
}

void BaseEngine::vma_initialization() {
  const VmaVulkanFunctions vulkan_functions{
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
  };

  const VmaAllocatorCreateInfo allocator_create_info{
      .physicalDevice = physical_device_,
      .device = device_,
      .pVulkanFunctions = &vulkan_functions,
      .instance = instance_,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };

  if (vmaCreateAllocator(&allocator_create_info, &allocator_) != VK_SUCCESS) {
    std::cerr << "Failed to create VMA allocator" << std::endl;
    return;
  }

  std::cout << "VMA allocator created successfully!" << std::endl;
}
