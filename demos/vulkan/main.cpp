#define VK_NO_PROTOTYPES
#include <volk.h>

#include <cstring>
#include <iostream>
#include <vector>

const char* vkResultToString(VkResult result) {
  switch (result) {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
      return "VK_ERROR_UNKNOWN";
    default:
      return "Unknown VkResult code";
  }
}

void check_vk_result(VkResult result, const char* message = "") {
  if (result != VK_SUCCESS) {
    std::cerr << "Vulkan Error (" << vkResultToString(result)
              << "): " << message << std::endl;
  }
}

int main() {
  check_vk_result(volkInitialize(), "Failed to initialize Volk");

  // Check for validation layer support
  uint32_t layerCount;
  check_vk_result(vkEnumerateInstanceLayerProperties(&layerCount, nullptr),
                  "Failed to get layer count");

  std::vector<VkLayerProperties> availableLayers(layerCount);
  check_vk_result(
      vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()),
      "Failed to enumerate instance layer properties");

  std::cout << "Available Layers:" << std::endl;
  for (const auto& layer : availableLayers) {
    std::cout << " - " << layer.layerName << std::endl;
    if (std::strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
      std::cout << "\n[Bingo!] Validation layer is available." << std::endl;
    }
  }

  // Check for available extensions
  uint32_t extensionCount = 0;
  check_vk_result(
      vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr),
      "Failed to get extension count");

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  check_vk_result(vkEnumerateInstanceExtensionProperties(
                      nullptr, &extensionCount, availableExtensions.data()),
                  "Failed to enumerate instance extension properties");

  // Initialize Vulkan instance with validation layer if available
  const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
  bool validationLayerAvailable = false;

  for (const auto& layer : availableLayers) {
    if (std::strcmp(layer.layerName, validationLayerName) == 0) {
      validationLayerAvailable = true;
      break;
    }
  }

  // list available extensions
  std::cout << "\nAvailable Extensions:" << std::endl;
  for (const auto& extension : availableExtensions) {
    std::cout << " - " << extension.extensionName << std::endl;
  }

  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "GPU Enumerator";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "No Engine";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (validationLayerAvailable) {
    createInfo.enabledLayerCount = 1;
    createInfo.ppEnabledLayerNames = &validationLayerName;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
  }

  VkInstance instance;
  check_vk_result(vkCreateInstance(&createInfo, nullptr, &instance),
                  "Failed to create Vulkan instance");
  volkLoadInstance(instance);

  // Enumerate physical devices (GPUs)
  uint32_t deviceCount = 0;
  check_vk_result(vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr),
                  "Failed to get physical device count");

  if (deviceCount == 0) {
    std::cerr << "No Vulkan-compatible GPUs found!" << std::endl;
    return -1;
  }

  std::vector<VkPhysicalDevice> devices(deviceCount);
  check_vk_result(
      vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()),
      "Failed to enumerate physical devices");

  // List GPU names
  std::cout << "\nFound " << deviceCount
            << " Vulkan-compatible GPU(s):" << std::endl;
  for (const auto& device : devices) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Check if this is a software device (llvmpipe)
    bool isSoftwareDevice =
        (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);

    if (isSoftwareDevice) {
      std::cout << "Found software Vulkan device: "
                << deviceProperties.deviceName << "\n"
                << std::endl;
      continue;  // Skip the detailed information for software devices
    }

    std::cout << "GPU: " << deviceProperties.deviceName << std::endl;
    std::cout << "API Version: "
              << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_MINOR(deviceProperties.apiVersion) << "."
              << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
    std::cout << "Driver Version: " << deviceProperties.driverVersion
              << std::endl;
    std::cout << "Vendor ID: " << deviceProperties.vendorID << std::endl;
    std::cout << "Device ID: " << deviceProperties.deviceID << std::endl;
    std::cout << "Device Type: ";
    switch (deviceProperties.deviceType) {
      case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        std::cout << "Integrated GPU";
        break;
      case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        std::cout << "Discrete GPU";
        break;
      case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        std::cout << "Virtual GPU";
        break;
      case VK_PHYSICAL_DEVICE_TYPE_CPU:
        std::cout << "CPU";
        break;
      default:
        std::cout << "Unknown";
        break;
    }
    std::cout << "\n" << std::endl;

    // Add Memory Properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    std::cout << "Memory Heaps:" << std::endl;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
      std::cout << "  Heap " << i << ": "
                << (memProperties.memoryHeaps[i].size /
                    (1024.0 * 1024.0 * 1024.0))
                << " GB";
      if (memProperties.memoryHeaps[i].flags &
          VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        std::cout << " (Device Local)";
      }
      std::cout << std::endl;
    }

    // Add Queue Family Properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data());

    std::cout << "\nQueue Families:" << std::endl;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      std::cout << "  Family " << i << ":" << std::endl;
      std::cout << "    Queue Count: " << queueFamilies[i].queueCount
                << std::endl;
      std::cout << "    Supported Operations: ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        std::cout << "Graphics ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        std::cout << "Compute ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        std::cout << "Transfer ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
        std::cout << "Sparse ";
      std::cout << std::endl;
    }

    // Add Device Limits (showing just a few important ones)
    std::cout << "\nDevice Limits:" << std::endl;
    std::cout << "  Max Compute Shared Memory Size: "
              << deviceProperties.limits.maxComputeSharedMemorySize / 1024.0
              << " KB" << std::endl;
    std::cout << "  Max Compute Work Group Count: "
              << deviceProperties.limits.maxComputeWorkGroupCount[0] << "x"
              << deviceProperties.limits.maxComputeWorkGroupCount[1] << "x"
              << deviceProperties.limits.maxComputeWorkGroupCount[2]
              << std::endl;
    std::cout << "  Max Compute Work Group Size: "
              << deviceProperties.limits.maxComputeWorkGroupSize[0] << "x"
              << deviceProperties.limits.maxComputeWorkGroupSize[1] << "x"
              << deviceProperties.limits.maxComputeWorkGroupSize[2]
              << std::endl;

    // Compute-specific Limits
    std::cout << "\nCompute-Specific Properties:" << std::endl;
    std::cout << "  Max Compute Work Group Invocations: "
              << deviceProperties.limits.maxComputeWorkGroupInvocations
              << std::endl;

    // Storage buffer limits
    std::cout << "  Max Storage Buffer Range: "
              << deviceProperties.limits.maxStorageBufferRange /
                     (1024.0 * 1024.0)
              << " MB" << std::endl;
    std::cout << "  Max Storage Buffer Bindings: "
              << deviceProperties.limits.maxPerStageDescriptorStorageBuffers
              << std::endl;

    // Timestamp and timing properties
    std::cout << "  Timestamp Period: "
              << deviceProperties.limits.timestampPeriod << " ns" << std::endl;

    // Get device features
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    std::cout << "\nRelevant Compute Features:" << std::endl;
    std::cout << "  64-bit Integers: "
              << (deviceFeatures.shaderInt64 ? "Yes" : "No") << std::endl;
    std::cout << "  64-bit Floats: "
              << (deviceFeatures.shaderFloat64 ? "Yes" : "No") << std::endl;
    std::cout << "  Variable Work Group Size: "
              << (deviceFeatures.shaderResourceResidency ? "Yes" : "No")
              << std::endl;

    // Get device extensions for compute
    uint32_t deviceExtCount;
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &deviceExtCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtCount);
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &deviceExtCount, deviceExtensions.data());

    std::cout << "\nRelevant Device Extensions:" << std::endl;
    for (const auto& ext : deviceExtensions) {
      // Filter for compute-relevant extensions
      if (strstr(ext.extensionName, "compute") != nullptr ||
          strstr(ext.extensionName, "shader") != nullptr ||
          strstr(ext.extensionName, "memory") != nullptr) {
        std::cout << "  " << ext.extensionName << std::endl;
      }
    }

    // Add a separator between devices
    std::cout << "\n----------------------------------------\n" << std::endl;
  }

  // Clean up
  vkDestroyInstance(instance, nullptr);

  return 0;
}
