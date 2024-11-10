#include "algorithm.hpp"

#include "shader_loader.hpp"
#include "vk_helper.hpp"

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

void Algorithm::create_descriptor_pool() {
  std::vector<VkDescriptorPoolSize> pool_sizes{
      {
          .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
          .descriptorCount = static_cast<uint32_t>(usm_buffers_.size()),
      },
  };

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
}

void Algorithm::create_descriptor_set_layout() {
  // for each buffer, need a binding
  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(usm_buffers_.size());

  for (size_t i = 0; i < usm_buffers_.size(); ++i) {
    bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = static_cast<uint32_t>(i),  // binding number need to match
                                              // the shader
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
}

void Algorithm::allocate_descriptor_sets() {
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
}

void Algorithm::update_descriptor_sets() {
  std::vector<VkWriteDescriptorSet> compute_write_descriptor_sets;
  compute_write_descriptor_sets.reserve(usm_buffers_.size());
  std::vector<VkDescriptorBufferInfo> buffer_infos(usm_buffers_.size());

  for (auto i = 0u; i < usm_buffers_.size(); ++i) {
    buffer_infos[i] = usm_buffers_[i]->construct_descriptor_buffer_info();
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
      device_,
      static_cast<uint32_t>(compute_write_descriptor_sets.size()),
      compute_write_descriptor_sets.data(),
      0,
      nullptr);
}

void Algorithm::set_push_constants(const void *data,
                                   const uint32_t size,
                                   const uint32_t type_memory_size) {
  spdlog::debug("Setting push constants with size: {} and type memory size: {}",
                size,
                type_memory_size);

  const uint32_t total_size = size * type_memory_size;

  push_constants_data_ = malloc(total_size);
  std::memcpy(push_constants_data_, data, total_size);

  push_constants_size_ = size;
  push_constants_data_type_memory_size_ = type_memory_size;
}

// void Algorithm::create_pipeline_layout() {
//   // Push constant range (1/3)
//   const auto push_constant_range = VkPushConstantRange{
//       .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
//       .offset = 0,
//       .size = push_constants_data_type_memory_size_ * push_constants_size_,
//   };

//   // Pipeline layout (2/3)
//   VkPipelineLayoutCreateInfo layout_create_info = {
//       .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
//       .setLayoutCount = 1,
//       .pSetLayouts = &descriptor_set_layout_,
//       .pushConstantRangeCount = 1,
//       .pPushConstantRanges = &push_constant_range,
//   };

//   // Create pipeline layout (3/3)
//   if (vkCreatePipelineLayout(
//           device_, &layout_create_info, nullptr, &pipeline_layout_) !=
//       VK_SUCCESS) {
//     throw std::runtime_error("Failed to create pipeline layout");
//   }
// }

// void Algorithm::create_pipeline_cache() {
//   // Pipeline cache info (1/2)
//   constexpr auto pipeline_cache_info = VkPipelineCacheCreateInfo{
//       .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
//   };

//   // Create pipeline cache (2/2)
//   if (vkCreatePipelineCache(
//           device_, &pipeline_cache_info, nullptr, &pipeline_cache_) !=
//       VK_SUCCESS) {
//     throw std::runtime_error("Failed to create pipeline cache");
//   }
// }

// void Algorithm::create_compute_pipeline() {
//   // Pipeline shader stage create info (1/3)
//   VkPipelineShaderStageCreateInfo shader_stage_create_info = {
//       .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//       .stage = VK_SHADER_STAGE_COMPUTE_BIT,
//       .module = shader_module_,
//       .pName = "main",
//   };

//   VkComputePipelineCreateInfo pipeline_create_info = {
//       .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
//       .stage = shader_stage_create_info,
//       .layout = pipeline_layout_,
//       .basePipelineHandle = VK_NULL_HANDLE,
//   };

//   check_vk_result(vkCreateComputePipelines(
//       device_, pipeline_cache_, 1, &pipeline_create_info, nullptr,
//       &pipeline_));
// }

void Algorithm::create_pipeline() {
  // Push constant Range (1.5/3)
  const auto push_constant_range = VkPushConstantRange{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = push_constants_data_type_memory_size_ * push_constants_size_,
  };

  // Pipeline layout (2/3)
  VkPipelineLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range,
  };

  if (vkCreatePipelineLayout(
          device_, &layout_create_info, nullptr, &pipeline_layout_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline layout");
  }

  // Pipeline cache (2.5/3)
  constexpr auto pipeline_cache_info = VkPipelineCacheCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
  };

  if (vkCreatePipelineCache(
          device_, &pipeline_cache_info, nullptr, &pipeline_cache_) !=
      VK_SUCCESS) {
    throw std::runtime_error("Failed to create pipeline cache");
  }

  // Specialization info (2.75/3) telling the shader the workgroup size

  spdlog::debug("Algorithm::create_pipeline, is not CLSPV shader");

  constexpr auto p_name = "main";

  // Pipeline itself (3/3)
  VkPipelineShaderStageCreateInfo shader_stage_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = shader_module_,
      .pName = p_name,
  };

  VkComputePipelineCreateInfo pipeline_create_info = {
      .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
      .stage = shader_stage_create_info,
      .layout = pipeline_layout_,
      .basePipelineHandle = VK_NULL_HANDLE,
  };

  spdlog::debug("Creating compute pipeline with:");
  spdlog::debug("  Shader module handle: {}", (void *)shader_module_);
  spdlog::debug("  Pipeline layout handle: {}", (void *)pipeline_layout_);
  spdlog::debug("  Entry point name: {}", p_name);

  // VkResult result = vkCreateComputePipelines(
  //     device_, pipeline_cache_, 1, &pipeline_create_info, nullptr,
  //     &pipeline_);
  // if (result != VK_SUCCESS) {
  //   throw std::runtime_error("Failed to create compute pipeline");
  // }

  check_vk_result(vkCreateComputePipelines(
      device_, pipeline_cache_, 1, &pipeline_create_info, nullptr, &pipeline_));

  spdlog::debug("Pipeline created successfully");
}

void Algorithm::create_shader_module() {
  const auto spirv_binary = load_shader_from_file(spirv_filename_);

  VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirv_binary.size(),
      .pCode = reinterpret_cast<const uint32_t *>(spirv_binary.data()),
  };

  check_vk_result(
      vkCreateShaderModule(device_, &create_info, nullptr, &shader_module_));

  spdlog::debug("Shader module created successfully");
}
