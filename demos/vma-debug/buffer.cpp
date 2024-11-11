#include "buffer.hpp"

#include <spdlog/spdlog.h>

#include "vk_helper.hpp"
#include "vk_mem_alloc.h"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

Buffer::Buffer(std::shared_ptr<VkDevice> device_ptr,
               const VkDeviceSize size,
               const VkBufferUsageFlags usage,
               const VmaMemoryUsage memory_usage,
               const VmaAllocationCreateFlags flags)
    : VulkanResource(std::move(device_ptr)),
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

  // if (const auto result = vmaCreateBuffer(BaseEngine::get_allocator(),
  //                                         &buffer_create_info,
  //                                         &memory_info,
  //                                         &this->handle_,
  //                                         &allocation_,
  //                                         &allocation_info);
  //     result != VK_SUCCESS) {
  //   throw std::runtime_error("Cannot create Buffer");
  // }

  check_vk_result(vmaCreateBuffer(BaseEngine::get_allocator(),
                                  &buffer_create_info,
                                  &memory_info,
                                  &this->handle_,
                                  &allocation_,
                                  &allocation_info));

  // log the allocation info
  spdlog::debug("\tsize: {}", allocation_info.size);
  spdlog::debug("\toffset: {}", allocation_info.offset);
  spdlog::debug("\tmemoryType: {}", allocation_info.memoryType);
  spdlog::debug("\tmappedData: {}", allocation_info.pMappedData);

  memory_ = static_cast<VkDeviceMemory>(allocation_info.deviceMemory);
  mapped_data_ = static_cast<std::byte *>(allocation_info.pMappedData);

  // // write something to the mapped data
  // std::memset(mapped_data_, 0x42, size);

  // // read something from the mapped data
  // spdlog::debug("mapped_data_[0]: {}", static_cast<int>(mapped_data_[0]));
}

void Buffer::destroy() {
  vmaDestroyBuffer(BaseEngine::get_allocator(), this->handle_, allocation_);
}

VkDescriptorBufferInfo Buffer::construct_descriptor_buffer_info() const {
  return VkDescriptorBufferInfo{
      .buffer = this->handle_,
      .offset = 0,
      .range = this->size_,
  };
}
