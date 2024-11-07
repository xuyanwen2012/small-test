
#include <iostream>

#include "base_engine.hpp"
#include "vk_mem_alloc.h"



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

  if (const auto result = vmaCreateBuffer(engine.get_allocator(),
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
  vmaDestroyBuffer(engine.get_allocator(), buffer, allocation_);

  return 0;
}
