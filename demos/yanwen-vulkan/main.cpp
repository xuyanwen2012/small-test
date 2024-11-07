#include <spdlog/spdlog.h>

#include "engine.hpp"
#include "third-party/CLI11.hpp"
#include "vma_usage.hpp"

int main(int argc, char** argv) {
  CLI::App app("yanwen-vulkan");

  std::string device_name;
  app.add_option("-d,--device", device_name, "Device name")
      ->required()
      ->default_val("jetson");

  bool debug = false;
  app.add_flag("--debug", debug, "Enable debug mode");

  app.parse(argc, argv);

  if (debug) {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::set_level(spdlog::level::info);
  }

  try {
    // Engine engine{};
    BaseEngine engine{};

    spdlog::debug("Creating buffer");
    Buffer buffer(engine.get_device(), 1024);
    spdlog::debug("Buffer created");

    // {
    //   const VkDeviceSize size = 1024;
    //   VkBuffer buffer = VK_NULL_HANDLE;
    //   std::byte* mapped_data_ = nullptr;

    //   const VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    //   const VmaMemoryUsage memory_usage = VMA_MEMORY_USAGE_AUTO;
    //   const VmaAllocationCreateFlags flags =
    //       VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
    //       VMA_ALLOCATION_CREATE_MAPPED_BIT;

    //   const VkBufferCreateInfo buffer_create_info{
    //       .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    //       .size = size,
    //       .usage = usage,
    //   };

    //   const VmaAllocationCreateInfo memory_info{
    //       .flags = flags,
    //       .usage = memory_usage,
    //   };

    //   VmaAllocation allocation_ = VK_NULL_HANDLE;
    //   VmaAllocationInfo allocation_info{};

    //   if (const auto result = vmaCreateBuffer(g_allocator,
    //                                           &buffer_create_info,
    //                                           &memory_info,
    //                                           &buffer,
    //                                           &allocation_,
    //                                           &allocation_info);
    //       result != VK_SUCCESS) {
    //     throw std::runtime_error("Cannot create HPPBuffer");
    //   }

    //   // log the allocation info
    //   std::cout << "Buffer::Buffer" << std::endl;
    //   std::cout << "\tsize: " << allocation_info.size << std::endl;
    //   std::cout << "\toffset: " << allocation_info.offset << std::endl;
    //   std::cout << "\tmemoryType: " << allocation_info.memoryType << std::endl;
    //   std::cout << "\tmappedData: " << allocation_info.pMappedData << std::endl;

    //   mapped_data_ = static_cast<std::byte*>(allocation_info.pMappedData);

    //   // write something to the mapped data
    //   std::memset(mapped_data_, 0x42, size);

    //   // read something from the mapped data
    //   std::cout << "mapped_data_[0]: " << static_cast<int>(mapped_data_[0])
    //             << std::endl;
    // }

  } catch (const std::exception& e) {
    spdlog::error("Error: {}", e.what());
    return EXIT_FAILURE;
  }

  return 0;
}