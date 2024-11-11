#define VK_NO_PROTOTYPES
#include <volk.h>

#include <cstring>
#include <iomanip>
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
  appInfo.apiVersion = VK_API_VERSION_1_1;

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

    // Get memory properties
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(device, &memProperties);

    // Get queue family properties
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount, queueFamilies.data());

    // Get device extensions
    uint32_t deviceExtCount;
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &deviceExtCount, nullptr);
    std::vector<VkExtensionProperties> deviceExtensions(deviceExtCount);
    vkEnumerateDeviceExtensionProperties(
        device, nullptr, &deviceExtCount, deviceExtensions.data());

    // Check if this is a software device (llvmpipe)
    bool isSoftwareDevice =
        (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU);

    if (isSoftwareDevice) {
      std::cout << "Found software Vulkan device: "
                << deviceProperties.deviceName << "\n"
                << std::endl;
      continue;  // Skip the detailed information for software devices
    }

    std::cout
        << "\n╔═══════════════════════════════════════════════════════════╗"
        << std::endl;
    std::cout
        << "║                    GPU INFORMATION                         ║"
        << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════╝"
              << std::endl;

    std::cout << "Device: " << deviceProperties.deviceName << std::endl;

    std::cout << "\nVersion Info:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "  │ API Version    : " << std::setw(2)
              << VK_VERSION_MAJOR(deviceProperties.apiVersion) << "."
              << std::setw(2) << VK_VERSION_MINOR(deviceProperties.apiVersion)
              << "." << std::setw(3)
              << VK_VERSION_PATCH(deviceProperties.apiVersion) << std::setw(35)
              << " │" << std::endl;
    std::cout << "  │ Driver Version : " << std::setw(10)
              << deviceProperties.driverVersion << std::setw(35) << " │"
              << std::endl;
    std::cout << "  │ Vendor ID      : " << std::setw(10)
              << deviceProperties.vendorID << std::setw(35) << " │"
              << std::endl;
    std::cout << "  │ Device ID      : " << std::setw(10)
              << deviceProperties.deviceID << std::setw(35) << " │"
              << std::endl;
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    std::cout << "\nDevice Type: ";
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
    std::cout << std::endl;

    std::cout << "\nMemory Information:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    for (uint32_t i = 0; i < memProperties.memoryHeapCount; i++) {
      float sizeGB =
          memProperties.memoryHeaps[i].size / (1024.0f * 1024.0f * 1024.0f);
      std::cout << "  │ Heap " << i << ": " << std::fixed
                << std::setprecision(2) << std::setw(6) << sizeGB << " GB";
      if (memProperties.memoryHeaps[i].flags &
          VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        std::cout << " (Device Local)";
      }
      std::cout << std::string(
                       std::max(0,
                                45 - static_cast<int>(
                                         std::to_string(sizeGB).length())),
                       ' ')
                << "│" << std::endl;
    }
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    std::cout << "\nQueue Families:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
      std::cout << "  │ Family " << std::left << std::setw(2) << i
                << ":                                            │"
                << std::endl;
      std::cout << "  │   Queues: " << std::left << std::setw(3)
                << queueFamilies[i].queueCount
                << "                                           │" << std::endl;
      std::cout << "  │   Operations: ";
      std::string ops;
      if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        ops += "Graphics, ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT)
        ops += "Compute, ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
        ops += "Transfer, ";
      if (queueFamilies[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
        ops += "Sparse, ";
      if (!ops.empty()) {
        ops = ops.substr(0, ops.length() - 2);  // Remove trailing comma
      }
      std::cout << std::left << std::setw(35) << ops << " │" << std::endl;
    }
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    std::cout << "\nCompute-Relevant Extensions:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    for (const auto& ext : deviceExtensions) {
      if (strstr(ext.extensionName, "compute") != nullptr ||
          strstr(ext.extensionName, "shader") != nullptr ||
          strstr(ext.extensionName, "memory") != nullptr) {
        std::string extName = ext.extensionName;
        if (extName.length() > 47) {
          extName = extName.substr(0, 44) + "...";
        }
        std::cout << "  │ " << std::left << std::setw(47) << extName << "│"
                  << std::endl;
      }
    }
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    std::cout
        << "\n╔═══════════════════════════════════════════════════════════╗"
        << std::endl;
    std::cout
        << "║                  COMPUTE CAPABILITIES                      ║"
        << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════════════╝"
              << std::endl;

    // Work Group Limits (equivalent to CUDA thread/block limits)
    std::cout << "\nWork Group Limits:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "  │ Max Work Group Size (threads per block):          │"
              << std::endl;
    std::cout << "  │   X: " << std::setw(5)
              << deviceProperties.limits.maxComputeWorkGroupSize[0]
              << "                                           │" << std::endl;
    std::cout << "  │   Y: " << std::setw(5)
              << deviceProperties.limits.maxComputeWorkGroupSize[1]
              << "                                           │" << std::endl;
    std::cout << "  │   Z: " << std::setw(5)
              << deviceProperties.limits.maxComputeWorkGroupSize[2]
              << "                                           │" << std::endl;
    std::cout << "  │                                                   │"
              << std::endl;
    std::cout << "  │ Max Work Group Count (grid dimensions):          │"
              << std::endl;
    std::cout << "  │   X: " << std::setw(10)
              << deviceProperties.limits.maxComputeWorkGroupCount[0]
              << "                                    │" << std::endl;
    std::cout << "  │   Y: " << std::setw(10)
              << deviceProperties.limits.maxComputeWorkGroupCount[1]
              << "                                    │" << std::endl;
    std::cout << "  │   Z: " << std::setw(10)
              << deviceProperties.limits.maxComputeWorkGroupCount[2]
              << "                                    │" << std::endl;
    std::cout << "  │                                                   │"
              << std::endl;
    std::cout << "  │ Max Total Threads Per Work Group: " << std::setw(5)
              << deviceProperties.limits.maxComputeWorkGroupInvocations
              << "                │" << std::endl;
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    // Memory and Storage Limits
    std::cout << "\nCompute Memory Limits:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "  │ Shared Memory Per Work Group: " << std::setw(6)
              << (deviceProperties.limits.maxComputeSharedMemorySize / 1024)
              << " KB            │" << std::endl;
    std::cout << "  │ Max Storage Buffer Range: " << std::setw(6)
              << (deviceProperties.limits.maxStorageBufferRange / (1024 * 1024))
              << " MB              │" << std::endl;
    std::cout << "  │ Max Storage Buffers Per Stage: " << std::setw(4)
              << deviceProperties.limits.maxPerStageDescriptorStorageBuffers
              << "                │" << std::endl;
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    // Get subgroup properties (similar to warp size in CUDA)
    VkPhysicalDeviceSubgroupProperties subgroupProperties = {};
    subgroupProperties.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;

    VkPhysicalDeviceProperties2 deviceProperties2 = {};
    deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    deviceProperties2.pNext = &subgroupProperties;

    vkGetPhysicalDeviceProperties2(device, &deviceProperties2);

    std::cout << "\nSubgroup Properties (equivalent to CUDA warp):"
              << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "  │ Subgroup Size: " << std::setw(4)
              << subgroupProperties.subgroupSize
              << " threads                           │" << std::endl;
    std::cout << "  │ Supported Operations:                             │"
              << std::endl;
    std::cout << "  │   Basic:      "
              << (subgroupProperties.supportedOperations &
                          VK_SUBGROUP_FEATURE_BASIC_BIT
                      ? "Yes"
                      : "No ")
              << "                                    │" << std::endl;
    std::cout << "  │   Vote:       "
              << (subgroupProperties.supportedOperations &
                          VK_SUBGROUP_FEATURE_VOTE_BIT
                      ? "Yes"
                      : "No ")
              << "                                    │" << std::endl;
    std::cout << "  │   Arithmetic: "
              << (subgroupProperties.supportedOperations &
                          VK_SUBGROUP_FEATURE_ARITHMETIC_BIT
                      ? "Yes"
                      : "No ")
              << "                                    │" << std::endl;
    std::cout << "  │   Ballot:     "
              << (subgroupProperties.supportedOperations &
                          VK_SUBGROUP_FEATURE_BALLOT_BIT
                      ? "Yes"
                      : "No ")
              << "                                    │" << std::endl;
    std::cout << "  │   Shuffle:    "
              << (subgroupProperties.supportedOperations &
                          VK_SUBGROUP_FEATURE_SHUFFLE_BIT
                      ? "Yes"
                      : "No ")
              << "                                    │" << std::endl;
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    // Add CUDA terminology reference
    std::cout << "\nCUDA Terminology Reference:" << std::endl;
    std::cout << "  ┌───────────────────────────────────────────────────┐"
              << std::endl;
    std::cout << "  │ Work Group         ═ Thread Block                  │"
              << std::endl;
    std::cout << "  │ Work Group Size    ═ Threads per Block            │"
              << std::endl;
    std::cout << "  │ Work Group Count   ═ Grid Dimensions              │"
              << std::endl;
    std::cout << "  │ Subgroup          ═ Warp                         │"
              << std::endl;
    std::cout << "  │ Shared Memory     ═ Shared Memory                │"
              << std::endl;
    std::cout << "  │ Storage Buffer    ═ Global Memory                │"
              << std::endl;
    std::cout << "  └───────────────────────────────────────────────────┘"
              << std::endl;

    // Add a separator between devices
    std::cout
        << "\n═══════════════════════════════════════════════════════════\n"
        << std::endl;
  }

  // Clean up
  vkDestroyInstance(instance, nullptr);

  return 0;
}
