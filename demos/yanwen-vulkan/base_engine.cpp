#include "base_engine.hpp"

#include <spdlog/spdlog.h>

// #include "vk_mem_alloc.h"

BaseEngine::BaseEngine(bool enable_validation)
    : enable_validation_(enable_validation) {
  spdlog::debug("BaseEngine::BaseEngine");

  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    throw std::runtime_error("Failed to initialize Volk");
  }

  initialize_device();
  vma_initialization();
}

void BaseEngine::destroy() {
  spdlog::debug("BaseEngine::destroy");

  // if (allocator_ != VK_NULL_HANDLE) {
  //   vmaDestroyAllocator(allocator_);
  //   allocator_ = VK_NULL_HANDLE;
  // }
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
  // Initialize Volk
  if (volkInitialize() != VK_SUCCESS) {
    throw std::runtime_error("Failed to initialize Volk");
  }


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

  // debug print all physical devices
  for (const auto& device : devices) {
    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(device, &device_properties);
    spdlog::info("Physical device: {}", device_properties.deviceName);
  }

  // just pick the first physical device, on android, there is only one physical
  // device
  VkPhysicalDevice physical_device = devices[0];

  if (physical_device == VK_NULL_HANDLE) {
    throw std::runtime_error(
        "Failed to find suitable GPU with compute capabilities");
  }

  // Create logical device
  uint32_t queue_family_index = 0;
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

  // that is this for? answer: load device functions
  volkLoadDevice(device_);

  // vkGetDeviceQueue(device_, queue_family_index, 0, &queue_);

  spdlog::info("Device created");
}

void BaseEngine::vma_initialization() {
  spdlog::debug("BaseEngine::vma_initialization");

  VmaVulkanFunctions vulkanFunctions = {};
  vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
  vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

  VmaAllocatorCreateInfo allocatorCreateInfo = {};
  allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
  allocatorCreateInfo.physicalDevice = physical_device_;
  allocatorCreateInfo.device = device_;
  allocatorCreateInfo.instance = instance_;
  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  VmaAllocator allocator;
  if (vmaCreateAllocator(&allocatorCreateInfo, &allocator) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create VMA allocator");
  }

  spdlog::info("VMA allocator created");

  vmaDestroyAllocator(allocator);
}
