#include <volk.h>

#include <iostream>
#include <vector>

#include "vk_mem_alloc.h"

class BaseEngine {
 public:
  BaseEngine() { initialize_device(); }
  ~BaseEngine() { destroy(); }

  void destroy();

 protected:
  void initialize_device();
  void vma_initialization();

  VkInstance instance_ = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device_ = VK_NULL_HANDLE;
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;

  // Vulkan Memory Allocator
  VmaAllocator allocator_ = VK_NULL_HANDLE;
};

void BaseEngine::destroy() {
  std::cout << "BaseEngine::destroy" << std::endl;
  // Cleanup
  vkDestroyDevice(device_, nullptr);
  vkDestroyInstance(instance_, nullptr);
  vmaDestroyAllocator(allocator_);
}

void BaseEngine::initialize_device() {
  std::cout << "BaseEngine::initialize_device" << std::endl;

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

  // VkDevice device;
  if (vkCreateDevice(physical_device_, &deviceCreateInfo, nullptr, &device_) !=
      VK_SUCCESS) {
    std::cerr << "Failed to create logical device" << std::endl;
    return;
  }

  volkLoadDevice(device_);

  std::cout << "Vulkan instance and device created successfully!" << std::endl;
}

void BaseEngine::vma_initialization() {
  std::cout << "BaseEngine::vma_initialization" << std::endl;

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
  if (vmaCreateAllocator(&allocatorCreateInfo, &allocator_) != VK_SUCCESS) {
    std::cerr << "Failed to create VMA allocator" << std::endl;
    return;
  }

  std::cout << "VMA allocator created successfully!" << std::endl;
}

int main() {
  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize Volk" << std::endl;
    return -1;
  }

  BaseEngine engine;

  return 0;
}
