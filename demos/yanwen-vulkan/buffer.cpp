#include "buffer.hpp"

#include "vma_usage.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

Buffer::Buffer(std::shared_ptr<VkDevice> device_ptr,
               const VkDeviceSize size,
               const VkBufferUsageFlags usage,
               const VmaMemoryUsage memory_usage,
               const VmaAllocationCreateFlags flags)
    : VulkanResource(std::move(device_ptr)),
      size_(size),
      persistent_{(flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) != 0} {
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

  if (const auto result = vmaCreateBuffer(
          g_allocator,
          reinterpret_cast<const VkBufferCreateInfo *>(&buffer_create_info),
          &memory_info,
          reinterpret_cast<VkBuffer *>(&get_handle()),
          &allocation_,
          &allocation_info);
      result != VK_SUCCESS) {
    throw std::runtime_error("Cannot create HPPBuffer");
  }

  // log the allocation info
  spdlog::debug("Buffer::Buffer");
  spdlog::debug("\tsize: {}", allocation_info.size);
  spdlog::debug("\toffset: {}", allocation_info.offset);
  spdlog::debug("\tmemoryType: {}", allocation_info.memoryType);
  spdlog::debug("\tmappedData: {}", allocation_info.pMappedData);

  memory_ = static_cast<VkDeviceMemory>(allocation_info.deviceMemory);

  if (persistent_) {
    mapped_data_ = static_cast<std::byte *>(allocation_info.pMappedData);
  }
}

Buffer::~Buffer() {
  spdlog::debug("Buffer::~Buffer");
  destroy();
}

void Buffer::destroy() {
  vmaDestroyBuffer(g_allocator, get_handle(), allocation_);
}
