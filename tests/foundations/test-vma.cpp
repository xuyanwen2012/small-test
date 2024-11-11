#define VK_NO_PROTOTYPES
#include <gtest/gtest.h>
#include <volk.h>

#include <iostream>
#include <vector>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

class VMATest : public ::testing::Test {
 protected:
  VkInstance instance = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  VkDevice device = VK_NULL_HANDLE;
  VmaAllocator allocator = VK_NULL_HANDLE;

  void SetUp() override {
    // Initialize Volk
    ASSERT_EQ(volkInitialize(), VK_SUCCESS) << "Failed to initialize Volk";

    // Create Vulkan instance
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "VMA Test";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    ASSERT_EQ(vkCreateInstance(&createInfo, nullptr, &instance), VK_SUCCESS)
        << "Failed to create Vulkan instance";

    // Load instance functions
    volkLoadInstance(instance);

    // Enumerate and select physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    ASSERT_GT(deviceCount, 0) << "No Vulkan-capable GPUs found";

    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());
    physicalDevice = physicalDevices[0];

    // Create logical device
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    ASSERT_EQ(
        vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device),
        VK_SUCCESS)
        << "Failed to create logical device";

    volkLoadDevice(device);

    // Create VMA allocator
    VmaVulkanFunctions vulkanFunctions = {};
    vulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo = {};
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_3;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    ASSERT_EQ(vmaCreateAllocator(&allocatorCreateInfo, &allocator), VK_SUCCESS)
        << "Failed to create VMA allocator";
  }

  void TearDown() override {
    if (allocator) vmaDestroyAllocator(allocator);
    if (device) vkDestroyDevice(device, nullptr);
    if (instance) vkDestroyInstance(instance, nullptr);
  }
};

TEST_F(VMATest, CreateAndMapBuffer) {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  const VkDeviceSize size = 1024;

  // Create buffer with host visible memory
  VkBufferCreateInfo bufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  };

  VmaAllocationCreateInfo allocationInfo{
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO,
  };

  VmaAllocationInfo allocationResult{};

  ASSERT_EQ(vmaCreateBuffer(allocator,
                            &bufferCreateInfo,
                            &allocationInfo,
                            &buffer,
                            &allocation,
                            &allocationResult),
            VK_SUCCESS)
      << "Failed to create buffer";

  // Verify allocation properties
  EXPECT_EQ(allocationResult.size, size);
  EXPECT_NE(allocationResult.pMappedData, nullptr);

  // Test writing to buffer
  auto* mappedData = static_cast<std::byte*>(allocationResult.pMappedData);
  std::memset(mappedData, 0x42, size);

  // Test reading from buffer
  EXPECT_EQ(static_cast<int>(mappedData[0]), 0x42);

  // Cleanup
  vmaDestroyBuffer(allocator, buffer, allocation);
}

TEST_F(VMATest, CreateDeviceLocalBuffer) {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  const VkDeviceSize size = 1024;

  VkBufferCreateInfo bufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  };

  VmaAllocationCreateInfo allocationInfo{
      .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };

  ASSERT_EQ(vmaCreateBuffer(allocator,
                            &bufferCreateInfo,
                            &allocationInfo,
                            &buffer,
                            &allocation,
                            nullptr),
            VK_SUCCESS)
      << "Failed to create device local buffer";

  // Cleanup
  vmaDestroyBuffer(allocator, buffer, allocation);
}

TEST_F(VMATest, MemoryBudget) {
  // Get memory budget for all heaps
  VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
  vmaGetHeapBudgets(allocator, budgets);

  // Verify that at least one heap has non-zero budget
  bool hasValidBudget = false;
  for (uint32_t i = 0; i < VK_MAX_MEMORY_HEAPS; ++i) {
    if (budgets[i].budget > 0) {
      hasValidBudget = true;
      break;
    }
  }
  EXPECT_TRUE(hasValidBudget);
}

TEST_F(VMATest, AllocationInfo) {
  VkBuffer buffer = VK_NULL_HANDLE;
  VmaAllocation allocation = VK_NULL_HANDLE;
  const VkDeviceSize size = 1024;

  VkBufferCreateInfo bufferInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      .size = size,
      .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  };

  VmaAllocationCreateInfo allocInfo{
      .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
      .usage = VMA_MEMORY_USAGE_AUTO,
  };

  ASSERT_EQ(
      vmaCreateBuffer(
          allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr),
      VK_SUCCESS);

  // Get allocation info
  VmaAllocationInfo allocInfo2{};
  vmaGetAllocationInfo(allocator, allocation, &allocInfo2);

  EXPECT_EQ(allocInfo2.size, size);

  // Get detailed statistics
  VmaTotalStatistics totalStats{};
  vmaCalculateStatistics(allocator, &totalStats);

  EXPECT_GT(totalStats.total.statistics.blockCount, 0u);
  EXPECT_GT(totalStats.total.statistics.blockBytes, 0ull);

  // Cleanup
  vmaDestroyBuffer(allocator, buffer, allocation);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
