#include "base_engine.hpp"

#include <vector>
#include <spdlog/spdlog.h>

#include "vma_usage.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// a single global allocator for the engine
VmaAllocator g_allocator = VK_NULL_HANDLE;

void BaseEngine::destroy() {
  spdlog::info("BaseEngine::destroy");
  // Cleanup
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
  vmaDestroyAllocator(g_allocator);
}

void BaseEngine::initialize_device() {
  spdlog::info("BaseEngine::initialize_device");

  // Initialize Volk [0/4]
  if (volkInitialize() != VK_SUCCESS) {
    throw std::runtime_error("Failed to initialize Volk");
  }

  // Create Vulkan instance [1/4]
  constexpr VkApplicationInfo appInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
      .pApplicationName = "Minimal Vulkan",
      .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
      .pEngineName = "No Engine",
      .engineVersion = VK_MAKE_VERSION(1, 0, 0),
      .apiVersion = VK_API_VERSION_1_3,
  };

  const VkInstanceCreateInfo createInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pApplicationInfo = &appInfo,
  };

  if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
    spdlog::error("Failed to create Vulkan instance");
    return;
  }

  // Load instance functions
  volkLoadInstance(instance_);

  // Enumerate physical devices [2/4]
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
  if (deviceCount == 0) {
    spdlog::error("Failed to find GPUs with Vulkan support");
    return;
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance_, &deviceCount, physicalDevices.data());

  // Select the first physical device [3/4]
  physical_device_ = physicalDevices[0];

  // print the name of the physical device
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physical_device_, &deviceProperties);
  spdlog::info("Physical device: {}", deviceProperties.deviceName);

  // Create a logical device [4/4]
  constexpr float queuePriority = 1.0f;
  const VkDeviceQueueCreateInfo queueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .queueFamilyIndex = 0,  // assume
      .queueCount = 1,
      .pQueuePriorities = &queuePriority,
  };

  const VkDeviceCreateInfo deviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
  };

  // VkDevice device;
  if (vkCreateDevice(physical_device_, &deviceCreateInfo, nullptr, &device_) !=
      VK_SUCCESS) {
    spdlog::error("Failed to create logical device");
    return;
  }

  volkLoadDevice(device_);

  spdlog::info("Vulkan instance and device created successfully!");
}

void BaseEngine::vma_initialization() {
  spdlog::info("BaseEngine::vma_initialization");

  const VmaVulkanFunctions vulkanFunctions{
      .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
      .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
  };

  const VmaAllocatorCreateInfo allocatorCreateInfo{
      .physicalDevice = physical_device_,
      .device = device_,
      .pVulkanFunctions = &vulkanFunctions,
      .instance = instance_,
      .vulkanApiVersion = VK_API_VERSION_1_3,
  };
  
  if (vmaCreateAllocator(&allocatorCreateInfo, &g_allocator) != VK_SUCCESS) {
    spdlog::error("Failed to create VMA allocator");
    return;
  }

  spdlog::info("VMA allocator created successfully!");
}