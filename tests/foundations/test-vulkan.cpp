#include <gtest/gtest.h>
#include <volk.h>

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <vulkan/vulkan.hpp>

class VulkanTest : public ::testing::Test {
 protected:
  vk::DynamicLoader dl;
  vk::Instance instance;
  vk::PhysicalDevice physicalDevice;
  std::vector<vk::PhysicalDevice> physicalDevices;
  vk::PhysicalDeviceProperties deviceProperties;
  vk::PhysicalDeviceFeatures deviceFeatures;
  std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
  vk::DispatchLoaderDynamic dispatch;

  void SetUp() override {
    // Initialize volk
    ASSERT_EQ(volkInitialize(), VK_SUCCESS) << "Failed to initialize volk!";

    // Initialize dynamic dispatch loader
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr =
        dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
    ASSERT_TRUE(vkGetInstanceProcAddr)
        << "Failed to get vkGetInstanceProcAddr!";
    dispatch = vk::DispatchLoaderDynamic(vkGetInstanceProcAddr);

    // Create Vulkan application info
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Test Vulkan")
        .setApplicationVersion(VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName("Test Engine")
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(VK_API_VERSION_1_0);

    // Enable validation layers in debug builds
    std::vector<const char*> validationLayers;
#ifdef _DEBUG
    validationLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    // Set up Vulkan instance create info
    vk::InstanceCreateInfo createInfo;
    createInfo.setPApplicationInfo(&appInfo)
        .setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()))
        .setPpEnabledLayerNames(validationLayers.data());

    // Create instance
    try {
      instance = vk::createInstance(createInfo, nullptr, dispatch);
      dispatch.init(instance);  // Initialize dispatch loader with instance
    } catch (const vk::SystemError& err) {
      FAIL() << "Failed to create Vulkan instance: " << err.what();
    }

    // Load instance-level function pointers with volk
    volkLoadInstance(instance);

    // Enumerate physical devices
    try {
      physicalDevices = instance.enumeratePhysicalDevices(dispatch);
    } catch (const vk::SystemError& err) {
      FAIL() << "Failed to enumerate physical devices: " << err.what();
    }

    if (!physicalDevices.empty()) {
      physicalDevice = physicalDevices[0];
      deviceProperties = physicalDevice.getProperties(dispatch);
      deviceFeatures = physicalDevice.getFeatures(dispatch);
      queueFamilyProperties = physicalDevice.getQueueFamilyProperties(dispatch);
    }
  }

  void TearDown() override {
    if (instance) {
      instance.destroy(nullptr, dispatch);
    }
  }
};

TEST_F(VulkanTest, InstanceCreation) {
  ASSERT_TRUE(instance) << "Vulkan instance is null!";
}

TEST_F(VulkanTest, PhysicalDeviceEnumeration) {
  ASSERT_FALSE(physicalDevices.empty()) << "No Vulkan physical devices found!";
  ASSERT_TRUE(physicalDevice) << "Failed to select physical device!";
}

TEST_F(VulkanTest, PhysicalDeviceProperties) {
  ASSERT_FALSE(std::string(deviceProperties.deviceName).empty())
      << "Device name is empty!";
  ASSERT_GT(deviceProperties.limits.maxImageDimension2D, 0u)
      << "Invalid max 2D image dimension!";
}

TEST_F(VulkanTest, QueueFamilyProperties) {
  ASSERT_FALSE(queueFamilyProperties.empty()) << "No queue families found!";

  auto hasGraphicsQueue = std::any_of(queueFamilyProperties.begin(),
                                      queueFamilyProperties.end(),
                                      [](const auto& queueFamily) {
                                        return queueFamily.queueFlags &
                                               vk::QueueFlagBits::eGraphics;
                                      });

  ASSERT_TRUE(hasGraphicsQueue) << "No graphics queue family found!";
}

TEST_F(VulkanTest, MemoryProperties) {
  auto memProperties = physicalDevice.getMemoryProperties(dispatch);

  ASSERT_GT(memProperties.memoryHeapCount, 0u) << "No memory heaps found!";
  ASSERT_GT(memProperties.memoryTypeCount, 0u) << "No memory types found!";
}

TEST_F(VulkanTest, DeviceExtensions) {
  try {
    auto extensions =
        physicalDevice.enumerateDeviceExtensionProperties(nullptr, dispatch);
    ASSERT_FALSE(extensions.empty()) << "No device extensions found!";
  } catch (const vk::SystemError& err) {
    FAIL() << "Failed to enumerate device extensions: " << err.what();
  }
}

TEST_F(VulkanTest, HasIntegratedGPU) {
  auto it = std::find_if(
      physicalDevices.begin(),
      physicalDevices.end(),
      [this](const vk::PhysicalDevice& device) {
        auto props = device.getProperties(dispatch);
        if (props.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
          std::cout << "Found integrated GPU: " << props.deviceName << "\n"
                    << "Driver version: " << props.driverVersion << "\n"
                    << "Vulkan API version: "
                    << VK_VERSION_MAJOR(props.apiVersion) << "."
                    << VK_VERSION_MINOR(props.apiVersion) << "."
                    << VK_VERSION_PATCH(props.apiVersion) << std::endl;
          return true;
        }
        return false;
      });

  bool hasIntegrated = (it != physicalDevices.end());
  EXPECT_TRUE(hasIntegrated) << "No integrated GPU found in the system";
}

TEST_F(VulkanTest, IntegratedGPUComputeSupport) {
  auto it = std::find_if(physicalDevices.begin(),
                         physicalDevices.end(),
                         [this](const vk::PhysicalDevice& device) {
                           return device.getProperties(dispatch).deviceType ==
                                  vk::PhysicalDeviceType::eIntegratedGpu;
                         });

  if (it != physicalDevices.end()) {
    auto queueProps = it->getQueueFamilyProperties(dispatch);

    auto computeQueueIt =
        std::find_if(queueProps.begin(),
                     queueProps.end(),
                     [](const vk::QueueFamilyProperties& props) {
                       return props.queueFlags & vk::QueueFlagBits::eCompute;
                     });

    bool hasComputeQueue = (computeQueueIt != queueProps.end());
    EXPECT_TRUE(hasComputeQueue)
        << "Integrated GPU does not support compute operations";

    if (hasComputeQueue) {
      auto computeQueueFamilyIndex =
          std::distance(queueProps.begin(), computeQueueIt);
      std::cout << "Integrated GPU supports compute operations\n"
                << "Compute queue family index: " << computeQueueFamilyIndex
                << "\n"
                << "Max compute queues: " << computeQueueIt->queueCount << "\n"
                << "Timestamp valid bits: "
                << computeQueueIt->timestampValidBits << std::endl;
    }
  } else {
    GTEST_SKIP() << "No integrated GPU found, skipping compute capability test";
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

