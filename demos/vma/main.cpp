#define VK_NO_PROTOTYPES
#include <volk.h>

#include <iostream>
#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

int main() {
  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize Volk" << std::endl;
    return -1;
  }

  // Create Vulkan instance
  VkInstance instance;
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "Minimal Vulkan";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
    std::cerr << "Failed to create Vulkan instance" << std::endl;
    return -1;
  }

  // Load instance functions
  volkLoadInstance(instance);

  // Enumerate physical devices
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
  if (deviceCount == 0) {
    std::cerr << "Failed to find GPUs with Vulkan support" << std::endl;
    return -1;
  }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

  // Select the first physical device
  VkPhysicalDevice physicalDevice = physicalDevices[0];
  // print the name of the physical device
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
  std::cout << "Physical device: " << deviceProperties.deviceName << std::endl;

  // Create a logical device
  float queuePriority = 1.0f;
  VkDeviceQueueCreateInfo queueCreateInfo = {};
  queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queueCreateInfo.queueFamilyIndex = 0;  // assume
  queueCreateInfo.queueCount = 1;
  queueCreateInfo.pQueuePriorities = &queuePriority;

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = 1;
  deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

  VkDevice device;
  if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create logical device" << std::endl;
    return -1;
  }

  volkLoadDevice(device);

  std::cout << "Vulkan instance and device created successfully!" << std::endl;

  // create VMA allocator
  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
  allocatorCreateInfo.physicalDevice = physicalDevice;
  allocatorCreateInfo.device = device;
  allocatorCreateInfo.instance = instance;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  VmaAllocator allocator;
  if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
    std::cerr << "Failed to create VMA allocator" << std::endl;
    return -1;
  }

  std::cout << "VMA allocator created successfully!" << std::endl;

  // ---------------------------------------------------------------------------
  VmaAllocation allocation_ = VK_NULL_HANDLE;
  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  const VkDeviceSize size = 1024;
  VkBuffer buffer = VK_NULL_HANDLE;

  const VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  const VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_CPU_ONLY;
  const VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

  const VkBufferCreateInfo buffer_create_info{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = usage,
  };

  const VmaAllocationCreateInfo memory_info{
      .flags = flags,
      .usage = memory_usage,
  };

  VmaAllocationInfo allocation_info{};

  if (const auto result = vmaCreateBuffer(
          allocator,
          reinterpret_cast<const VkBufferCreateInfo *>(&buffer_create_info),
          &memory_info,
          // reinterpret_cast<VkBuffer *>(&buffer),
          &buffer,
          &allocation_,
          &allocation_info);
      result != VK_SUCCESS) {
    throw std::runtime_error("Cannot create HPPBuffer");
  }

  // log the allocation info
  std::cout << "Buffer::Buffer" << std::endl;
  std::cout << "\tsize: " << allocation_info.size << std::endl;
  std::cout << "\toffset: " << allocation_info.offset << std::endl;
  std::cout << "\tmemoryType: " << allocation_info.memoryType << std::endl;
  std::cout << "\tmappedData: " << allocation_info.pMappedData << std::endl;

  // ---------------------------------------------------------------------------

  // Cleanup
  vmaDestroyBuffer(allocator, buffer, allocation_);
  vmaDestroyAllocator(allocator);
  
  vkDestroyDevice(device, nullptr);
  vkDestroyInstance(instance, nullptr);
  return 0;
}
