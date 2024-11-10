#include "buffer.hpp"

#include <spdlog/spdlog.h>

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

Buffer::Buffer(VkDevice device,
               const VkDeviceSize size,
               const VkBufferUsageFlags usage,
               const VmaMemoryUsage memory_usage,
               const VmaAllocationCreateFlags flags)
    : device_(device),
      size_(size),
      persistent_{(flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0} {
  spdlog::debug("Buffer::Buffer");

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

  if (const auto result = vmaCreateBuffer(BaseEngine::get_allocator(),
                                          &buffer_create_info,
                                          &memory_info,
                                          &buffer_,
                                          &allocation_,
                                          &allocation_info);
      result != VK_SUCCESS) {
    throw std::runtime_error("Cannot create HPPBuffer");
  }

  // log the allocation info
  spdlog::info("\tsize: {}", allocation_info.size);
  spdlog::info("\toffset: {}", allocation_info.offset);
  spdlog::info("\tmemoryType: {}", allocation_info.memoryType);
  spdlog::info("\tmappedData: {}", allocation_info.pMappedData);

  memory_ = static_cast<VkDeviceMemory>(allocation_info.deviceMemory);
  mapped_data_ = static_cast<std::byte *>(allocation_info.pMappedData);

  // write something to the mapped data
  std::memset(mapped_data_, 0x42, size);

  // read something from the mapped data
  spdlog::info("mapped_data_[0]: {}", static_cast<int>(mapped_data_[0]));
}

Buffer::~Buffer() {
  spdlog::debug("Buffer::~Buffer");
  destroy();
}

void Buffer::destroy() {
  vmaDestroyBuffer(BaseEngine::get_allocator(), buffer_, allocation_);
}

VkDescriptorBufferInfo Buffer::construct_descriptor_buffer_info() const {
  return VkDescriptorBufferInfo{
      .buffer = buffer_,
      .offset = 0,
      .range = size_,
  };
}
