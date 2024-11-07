#define VK_NO_PROTOTYPES
#include <volk.h>

#include <iostream>
#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// -----------------------------------------------------------------------------
// global allocator
// -----------------------------------------------------------------------------

VmaAllocator g_allocator = VK_NULL_HANDLE;

// -----------------------------------------------------------------------------
// BaseEngine.hpp
// -----------------------------------------------------------------------------

class BaseEngine {
 public:
  BaseEngine() {
    initialize_device();
    vma_initialization();
  }
  ~BaseEngine() { destroy(); }

  void destroy();

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
};

// -----------------------------------------------------------------------------
// BaseEngine.cpp
// -----------------------------------------------------------------------------

void BaseEngine::destroy() {
  vmaDestroyAllocator(g_allocator);
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
}

void BaseEngine::initialize_device() {
  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize Volk" << std::endl;
    return;
  }

  // Create Vulkan instance
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

  if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
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

  // Select the first physical device
  physical_device_ = physicalDevices[0];
  // print the name of the physical device
  VkPhysicalDeviceProperties deviceProperties;
  vkGetPhysicalDeviceProperties(physical_device_, &deviceProperties);
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

  if (vkCreateDevice(physical_device_, &deviceCreateInfo, nullptr, &device_) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create logical device" << std::endl;
    return;
  }

  volkLoadDevice(device_);

  std::cout << "Vulkan instance and device created successfully!" << std::endl;
}

void BaseEngine::vma_initialization() {
  // create VMA allocator
  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
  allocatorCreateInfo.physicalDevice = physical_device_;
  allocatorCreateInfo.device = device_;
  allocatorCreateInfo.instance = instance_;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  // VmaAllocator allocator;
  if (vmaCreateAllocator(&allocatorCreateInfo, &g_allocator) != VK_SUCCESS) {
    std::cerr << "Failed to create VMA allocator" << std::endl;
    return;
  }

  std::cout << "VMA allocator created successfully!" << std::endl;
}

// -----------------------------------------------------------------------------
// main
// -----------------------------------------------------------------------------

int main() {
  BaseEngine engine;

  // ---------------------------------------------------------------------------
  VkBuffer buffer = VK_NULL_HANDLE;

  VmaAllocation allocation_ = VK_NULL_HANDLE;
  const VkDeviceSize size = 1024;

  VkDeviceMemory memory_ = VK_NULL_HANDLE;
  std::byte *mapped_data_ = nullptr;

  const VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  const VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO;
  const VmaAllocationCreateFlags flags =
      VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
      VMA_ALLOCATION_CREATE_MAPPED_BIT;

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

  if (const auto result = vmaCreateBuffer(g_allocator,
                                          &buffer_create_info,
                                          &memory_info,
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

  memory_ = static_cast<VkDeviceMemory>(allocation_info.deviceMemory);
  mapped_data_ = static_cast<std::byte *>(allocation_info.pMappedData);

  // write something to the mapped data
  std::memset(mapped_data_, 0x42, size);

  // read something from the mapped data
  std::cout << "mapped_data_[0]: " << static_cast<int>(mapped_data_[0])
            << std::endl;

  // Cleanup
  vmaDestroyBuffer(g_allocator, buffer, allocation_);

  return 0;
}
