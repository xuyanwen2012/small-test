#include "vulkan/algorithm.hpp"

#include "vulkan/shader_loader.hpp"
#include "vulkan/vk_helper.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

void Algorithm::destroy() {
  spdlog::debug("Algorithm::destroy()");

  vkDestroyShaderModule(*device_ptr_, handle_, nullptr);
  vkDestroyPipeline(*device_ptr_, pipeline_, nullptr);
  vkDestroyPipelineCache(*device_ptr_, pipeline_cache_, nullptr);
  vkDestroyPipelineLayout(*device_ptr_, pipeline_layout_, nullptr);
  vkDestroyDescriptorSetLayout(*device_ptr_, descriptor_set_layout_, nullptr);
  vkDestroyDescriptorPool(*device_ptr_, descriptor_pool_, nullptr);
}

void Algorithm::create_descriptor_pool() {
  const std::vector<VkDescriptorPoolSize> pool_sizes = {
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = static_cast<uint32_t>(usm_buffers_.size()),
      },
  };

  const VkDescriptorPoolCreateInfo pool_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .maxSets = 1,
      .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
      .pPoolSizes = pool_sizes.data(),
  };

  check_vk_result(vkCreateDescriptorPool(
      *device_ptr_, &pool_info, nullptr, &descriptor_pool_));
}

void Algorithm::create_descriptor_set_layout() {
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

  const VkDescriptorSetLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  check_vk_result(vkCreateDescriptorSetLayout(
      *device_ptr_, &layout_create_info, nullptr, &descriptor_set_layout_));
}

void Algorithm::allocate_descriptor_sets() {
  const VkDescriptorSetAllocateInfo set_allocate_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
      .descriptorPool = descriptor_pool_,
      .descriptorSetCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
  };

  check_vk_result(vkAllocateDescriptorSets(
      *device_ptr_, &set_allocate_info, &descriptor_set_));
}

void Algorithm::create_pipeline() {
  const VkPushConstantRange push_constant_range = {
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = push_constants_size_,
  };

  const VkPipelineLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
      .pushConstantRangeCount =
          static_cast<uint32_t>(has_push_constants() ? 1 : 0),
      .pPushConstantRanges = &push_constant_range,
  };

  check_vk_result(vkCreatePipelineLayout(
      *device_ptr_, &layout_create_info, nullptr, &pipeline_layout_));

  constexpr VkPipelineCacheCreateInfo pipeline_cache_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
  };

  check_vk_result(vkCreatePipelineCache(
      *device_ptr_, &pipeline_cache_info, nullptr, &pipeline_cache_));

  constexpr auto p_name = "main";

  const VkPipelineShaderStageCreateInfo shader_stage_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = this->get_handle(),
      .pName = p_name,
  };

  const VkComputePipelineCreateInfo pipeline_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = shader_stage_create_info,
      .layout = pipeline_layout_,
      .basePipelineHandle = VK_NULL_HANDLE,
  };

  spdlog::debug("Creating compute pipeline with:");
  spdlog::debug("  Shader module handle: {}", (void *)this->get_handle());
  spdlog::debug("  Pipeline layout handle: {}", (void *)pipeline_layout_);
  spdlog::debug("  Entry point name: {}", p_name);

  check_vk_result(vkCreateComputePipelines(*device_ptr_,
                                           pipeline_cache_,
                                           1,
                                           &pipeline_create_info,
                                           nullptr,
                                           &pipeline_));

  spdlog::debug("Pipeline created successfully");
}

void Algorithm::create_shader_module() {
  const auto spirv_binary = load_shader_from_file(spirv_filename_);

  const VkShaderModuleCreateInfo create_info = {
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirv_binary.size() * sizeof(uint32_t),
      .pCode = reinterpret_cast<const uint32_t *>(spirv_binary.data()),
  };

  check_vk_result(vkCreateShaderModule(
      *device_ptr_, &create_info, nullptr, &this->get_handle()));

  spdlog::debug("Shader module created successfully");
}

// this method send the buffer data to the shader
void Algorithm::record_bind_core(VkCommandBuffer cmd_buf) const {
  spdlog::debug("Algorithm::record_bind_core()");

  vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
  vkCmdBindDescriptorSets(cmd_buf,
                          VK_PIPELINE_BIND_POINT_COMPUTE,
                          pipeline_layout_,
                          0,
                          1,
                          &descriptor_set_,
                          0,
                          nullptr);
}

// this method send the push constants to the shader
void Algorithm::record_bind_push(VkCommandBuffer cmd_buf) const {
  spdlog::debug("Algorithm::record_bind_push, constants memory size: {}",
                push_constants_size_);

  if (push_constants_size_ == 0) {
    return;
  }

  vkCmdPushConstants(cmd_buf,
                     pipeline_layout_,
                     VK_SHADER_STAGE_COMPUTE_BIT,
                     0,
                     push_constants_size_,
                     push_constants_data_.data());
}

// this method dispatch the kernel with the number of blocks provided.
void Algorithm::record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
                                            uint32_t n_blocks) const {
  spdlog::debug("Algorithm::record_dispatch_with_blocks, n_blocks: {}",
                n_blocks);

  vkCmdDispatch(cmd_buf, n_blocks, 1u, 1u);
}

void Algorithm::update_descriptor_sets() {
  update_descriptor_sets_with_buffers(usm_buffers_);
  //   std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
  //   compute_write_descriptor_sets.reserve(usm_buffers_.size());
  //   std::vector<VkDescriptorBufferInfo> buffer_infos(usm_buffers_.size());

  //   for (auto i = 0u; i < usm_buffers_.size(); ++i) {
  //     buffer_infos[i] = usm_buffers_[i]->construct_descriptor_buffer_info();
  //     compute_write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
  //         .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
  //         .dstSet = descriptor_set_,
  //         .dstBinding = static_cast<uint32_t>(i),
  //         .dstArrayElement = 0,
  //         .descriptorCount = 1,
  //         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
  //         .pBufferInfo = &buffer_infos[i],
  //     });
  //   }

  //   vkUpdateDescriptorSets(
  //       *device_ptr_,
  //       static_cast<uint32_t>(compute_write_descriptor_sets.size()),
  //       compute_write_descriptor_sets.data(),
  //       0,
  //       nullptr);
}

// this method update the descriptor sets with the buffers provided.
void Algorithm::update_descriptor_sets_with_buffers(
    const std::vector<std::shared_ptr<Buffer>> &buffers) {
      
  std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
  compute_write_descriptor_sets.reserve(buffers.size());
  std::vector<VkDescriptorBufferInfo> buffer_infos(buffers.size());

  for (auto i = 0u; i < buffers.size(); ++i) {
    buffer_infos[i] = buffers[i]->construct_descriptor_buffer_info();
    compute_write_descriptor_sets.emplace_back(VkWriteDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptor_set_,
        .dstBinding = static_cast<uint32_t>(i),
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .pBufferInfo = &buffer_infos[i],
    });
  }

  vkUpdateDescriptorSets(
      *device_ptr_,
      static_cast<uint32_t>(compute_write_descriptor_sets.size()),
      compute_write_descriptor_sets.data(),
      0,
      nullptr);
}
