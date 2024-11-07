#include "algorithm.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

void Algorithm::destroy() {
  vkDestroyShaderModule(device_, shader_module_, nullptr);
  vkDestroyPipeline(device_, pipeline_, nullptr);
  vkDestroyPipelineCache(device_, pipeline_cache_, nullptr);
  vkDestroyPipelineLayout(device_, pipeline_layout_, nullptr);
  vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);
  vkDestroyDescriptorPool(device_, descriptor_pool_, nullptr);
  free(push_constants_data_);
}

void Algorithm::create_parameters() {
  // Pool size
  std::vector<VkDescriptorPoolSize> pool_sizes{
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = static_cast<uint32_t>(usm_buffers_.size()),
      },
  };

  // Descriptor pool
  VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
      .pPoolSizes = pool_sizes.data(),
  };

  if (vkCreateDescriptorPool(device_, &pool_info, nullptr, &descriptor_pool_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor pool");
  }

  // Descriptor set
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(usm_buffers_.size());
  for (size_t i = 0; i < usm_buffers_.size(); ++i) {
    bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = static_cast<uint32_t>(i),
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
    });
  }

  VkDescriptorSetLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  if (vkCreateDescriptorSetLayout(
          device_, &layout_create_info, nullptr, &descriptor_set_layout_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create descriptor set layout");
  }

  // Allocate descriptor set
  VkDescriptorSetAllocateInfo set_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool_,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
  };

  if (vkAllocateDescriptorSets(device_, &set_allocate_info, &descriptor_set_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to allocate descriptor set");
  }

  // Update descriptor set
  for (auto i = 0u; i < usm_buffers_.size(); ++i) {
    std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
    compute_write_descriptor_sets.reserve(usm_buffers_.size());

    auto buf_info = usm_buffers_[i]->construct_descriptor_buffer_info();
    compute_write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set_,
        .dstBinding = static_cast<uint32_t>(i),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .pBufferInfo = &buf_info,
    });

    spdlog::debug("Updating descriptor set {}", i);
    vkUpdateDescriptorSets(
        device_,
        static_cast<uint32_t>(compute_write_descriptor_sets.size()),
        compute_write_descriptor_sets.data(),
        0,
        nullptr);
  }
}

void Algorithm::create_pipeline() {}

void Algorithm::create_shader_module() {}
