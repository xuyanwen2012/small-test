#include "vulkan/algorithm.hpp"

#include "vulkan/shader_loader.hpp"
#include "vulkan/vk_helper.hpp"

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

// ---------------------------------------------------------------------------
// Destruction
// ---------------------------------------------------------------------------

void Algorithm::destroy() {
  spdlog::debug("Algorithm::destroy()");

  vkDestroyShaderModule(*device_ptr_, handle_, nullptr);

  vkDestroyPipeline(*device_ptr_, pipeline_, nullptr);
  vkDestroyPipelineCache(*device_ptr_, pipeline_cache_, nullptr);
  vkDestroyPipelineLayout(*device_ptr_, pipeline_layout_, nullptr);
  vkDestroyDescriptorSetLayout(*device_ptr_, descriptor_set_layout_, nullptr);
  vkDestroyDescriptorPool(*device_ptr_, descriptor_pool_, nullptr);

  spvReflectDestroyShaderModule(&reflect_module_);
}

// ---------------------------------------------------------------------------
// Shader module
// ---------------------------------------------------------------------------

void Algorithm::create_shader_module() {
  const auto spirv_binary = load_shader_from_file(spirv_filename_);

  // Reflect the shader module before anything else
  if (spvReflectCreateShaderModule(spirv_binary.size() * sizeof(uint32_t),
                                   spirv_binary.data(),
                                   &reflect_module_) !=
      SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to reflect shader module");
    return;
  }

  // We can create the shader module now, it does not need the reflection data
  const VkShaderModuleCreateInfo create_info{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = spirv_binary.size() * sizeof(uint32_t),
      .pCode = reinterpret_cast<const uint32_t *>(spirv_binary.data()),
  };

  check_vk_result(vkCreateShaderModule(
      *device_ptr_, &create_info, nullptr, &this->get_handle()));

  spdlog::debug("Shader module created successfully");
}

// ---------------------------------------------------------------------------
// Descriptor set layout
// ---------------------------------------------------------------------------

void Algorithm::create_descriptor_set_layout() {

  // Query descriptor set count and layouts
  uint32_t descriptor_set_count = 0;
  if (spvReflectEnumerateDescriptorSets(
          &reflect_module_, &descriptor_set_count, nullptr) !=
      SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate descriptor sets.");
    return;
  }

  // for my application, I only have one descriptor set
  assert(descriptor_set_count == 1);

  // Query descriptor set layouts
  std::vector<SpvReflectDescriptorSet *> descriptor_sets(descriptor_set_count);
  spvReflectEnumerateDescriptorSets(
      &reflect_module_, &descriptor_set_count, descriptor_sets.data());

  std::vector<VkDescriptorSetLayoutBinding> bindings;
  bindings.reserve(descriptor_sets[0]->binding_count);

  for (uint32_t i = 0; i < descriptor_set_count; ++i) {
    const auto& set = descriptor_sets[i];

    for (uint32_t j = 0; j < set->binding_count; ++j) {
      const auto& binding = set->bindings[j];
      
      bindings.push_back(VkDescriptorSetLayoutBinding{
          .binding = binding->binding,
          .descriptorType = static_cast<VkDescriptorType>(binding->descriptor_type),
          .descriptorCount = 1, // Or binding->count if it's an array
          .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
          .pImmutableSamplers = nullptr
      });
    }
  }

  VkDescriptorSetLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
      .bindingCount = static_cast<uint32_t>(bindings.size()),
      .pBindings = bindings.data(),
  };

  check_vk_result(vkCreateDescriptorSetLayout(
      *device_ptr_, &layout_create_info, nullptr, &descriptor_set_layout_));
}

// ---------------------------------------------------------------------------
// Descriptor pool
// ---------------------------------------------------------------------------

void Algorithm::create_descriptor_pool() {
    // Query descriptor set count and layouts (we already know it's 1 from earlier assert)
    std::vector<SpvReflectDescriptorSet*> descriptor_sets(1);
    uint32_t descriptor_set_count = 1;
    spvReflectEnumerateDescriptorSets(&reflect_module_, &descriptor_set_count, descriptor_sets.data());
    
    const auto& set = descriptor_sets[0];
    
    // Count how many descriptors of each type we need
    std::unordered_map<VkDescriptorType, uint32_t> type_counts;
    for (uint32_t i = 0; i < set->binding_count; i++) {
        const auto& binding = set->bindings[i];
        VkDescriptorType type = static_cast<VkDescriptorType>(binding->descriptor_type);
        type_counts[type] += binding->count; // binding->count handles arrays
    }
    
    // Create pool sizes array from the counts
    std::vector<VkDescriptorPoolSize> pool_sizes;
    pool_sizes.reserve(type_counts.size());
    for (const auto& [type, count] : type_counts) {
        pool_sizes.push_back(VkDescriptorPoolSize{
            .type = type,
            .descriptorCount = count
        });
    }

    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,  // We only need one descriptor set
        .poolSizeCount = static_cast<uint32_t>(pool_sizes.size()),
        .pPoolSizes = pool_sizes.data(),
    };

    check_vk_result(vkCreateDescriptorPool(
        *device_ptr_, &pool_info, nullptr, &descriptor_pool_));
}

void Algorithm::allocate_descriptor_sets() {
    VkDescriptorSetAllocateInfo set_allocate_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptor_pool_,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptor_set_layout_,
    };

    check_vk_result(vkAllocateDescriptorSets(
        *device_ptr_, &set_allocate_info, &descriptor_set_));
}

void Algorithm::update_descriptor_sets() {
    // Get descriptor set information from reflection
    std::vector<SpvReflectDescriptorSet*> descriptor_sets(1);
    uint32_t descriptor_set_count = 1;
    spvReflectEnumerateDescriptorSets(&reflect_module_, &descriptor_set_count, descriptor_sets.data());
    
    const auto& set = descriptor_sets[0];
    
    // Prepare descriptor writes
    std::vector<VkWriteDescriptorSet> descriptor_writes;
    std::vector<VkDescriptorBufferInfo> buffer_infos;
    
    descriptor_writes.reserve(set->binding_count);
    buffer_infos.reserve(set->binding_count);

    for (uint32_t i = 0; i < set->binding_count; i++) {
        const auto& binding = set->bindings[i];
        
        // Skip if not a storage buffer
        if (binding->descriptor_type != SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            spdlog::warn("Skipping non-storage buffer binding {}", binding->binding);
            continue;
        }

        // Get the buffer for this binding
        auto it = storage_buffers_.find(binding->binding);
        if (it == storage_buffers_.end()) {
            spdlog::warn("No buffer registered for binding {}", binding->binding);
            continue;
        }

        buffer_infos.push_back(it->second->construct_descriptor_buffer_info());
        
        descriptor_writes.push_back(VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptor_set_,
            .dstBinding = binding->binding,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &buffer_infos.back()
        });
    }

    if (!descriptor_writes.empty()) {
        vkUpdateDescriptorSets(
            *device_ptr_,
            static_cast<uint32_t>(descriptor_writes.size()),
            descriptor_writes.data(),
            0,
            nullptr);
    }
}

void Algorithm::create_pipeline() {
  // reflect
  uint32_t push_constant_count = 0;
  if (spvReflectEnumeratePushConstantBlocks(
          &reflect_module_, &push_constant_count, nullptr) !=
      SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("spvReflect:Failed to enumerate push constant blocks.");
    return;
  }

  assert(push_constant_count == push_constants_count_);

  std::vector<SpvReflectBlockVariable *> push_constants(push_constant_count);
  if (push_constant_count > 0) {
    spdlog::info("Push constant count: {}", push_constant_count);
    if (spvReflectEnumeratePushConstantBlocks(
            &reflect_module_, &push_constant_count, push_constants.data()) !=
        SPV_REFLECT_RESULT_SUCCESS) {
      spdlog::error("spvReflect:Failed to get push constant blocks.");
      return;
    }
  }

  auto push_constants_size = push_constants[0]->size;
  push_constants_size_ = push_constants_size;
  spdlog::info("Reflected: Push constants size: {}", push_constants_size);

  // ---------------------------------------------------------------------------
  // Push constant Range (1.5/3)
  // TODO: these should match the reflected result

  const auto push_constant_range = VkPushConstantRange{
      .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
      .offset = 0,
      .size = push_constants_size,
  };

  // Pipeline layout (2/3)
  VkPipelineLayoutCreateInfo layout_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .setLayoutCount = 1,
      .pSetLayouts = &descriptor_set_layout_,
      .pushConstantRangeCount = 1,
      .pPushConstantRanges = &push_constant_range,
  };

  check_vk_result(vkCreatePipelineLayout(
      *device_ptr_, &layout_create_info, nullptr, &pipeline_layout_));

  // Pipeline cache (2.5/3)
  constexpr auto pipeline_cache_info = VkPipelineCacheCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
  };

  check_vk_result(vkCreatePipelineCache(
      *device_ptr_, &pipeline_cache_info, nullptr, &pipeline_cache_));

  // Specialization info (2.75/3) telling the shader the workgroup size
  // constexpr auto p_name = "main";
  const auto p_name = reflect_module_.entry_point_name;

  // Pipeline itself (3/3)
  VkPipelineShaderStageCreateInfo shader_stage_create_info = {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = VK_SHADER_STAGE_COMPUTE_BIT,
      .module = this->get_handle(),
      .pName = p_name,
  };

  VkComputePipelineCreateInfo pipeline_create_info = {
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

// // this method send the buffer data to the shader
// void Algorithm::record_bind_core(VkCommandBuffer cmd_buf) const {
//   spdlog::debug("Algorithm::record_bind_core()");

//   vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
//   vkCmdBindDescriptorSets(cmd_buf,
//                           VK_PIPELINE_BIND_POINT_COMPUTE,
//                           pipeline_layout_,
//                           0,
//                           1,
//                           &descriptor_set_,
//                           0,
//                           nullptr);
// }

// // this method send the push constants to the shader
// void Algorithm::record_bind_push(VkCommandBuffer cmd_buf) const {
//   spdlog::debug("Algorithm::record_bind_push, constants memory size: {}",
//                 heterogeneous_push_constants_size_);

//   vkCmdPushConstants(cmd_buf,
//                      pipeline_layout_,
//                      VK_SHADER_STAGE_COMPUTE_BIT,
//                      0,
//                      heterogeneous_push_constants_size_,
//                      heterogeneous_push_constants_data_);
// }

// // this method dispatch the kernel with the number of blocks provided.
// void Algorithm::record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
//                                             uint32_t n_blocks) const {
//   spdlog::debug("Algorithm::record_dispatch_with_blocks, n_blocks: {}",
//                 n_blocks);

//   vkCmdDispatch(cmd_buf, n_blocks, 1u, 1u);
// }

// void Algorithm::record_bind_core(VkCommandBuffer cmd_buf) const {
//   spdlog::debug("Algorithm::record_bind_core()");

//   vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline_);
//   vkCmdBindDescriptorSets(cmd_buf,
//                          VK_PIPELINE_BIND_POINT_COMPUTE,
//                          pipeline_layout_,
//                          0,
//                          1,
//                          &descriptor_set_,
//                          0,
//                          nullptr);
// }

// void Algorithm::record_bind_push(VkCommandBuffer cmd_buf) const {
//   if (heterogeneous_push_constants_size_ > 0 && heterogeneous_push_constants_data_ != nullptr) {
//     spdlog::debug("Algorithm::record_bind_push, constants memory size: {}",
//                   heterogeneous_push_constants_size_);

//     vkCmdPushConstants(cmd_buf,
//                       pipeline_layout_,
//                       VK_SHADER_STAGE_COMPUTE_BIT,
//                       0,
//                       heterogeneous_push_constants_size_,
//                       heterogeneous_push_constants_data_);
//   }
// }

// void Algorithm::record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
//                                           uint32_t n_blocks) const {
//   spdlog::debug("Algorithm::record_dispatch_with_blocks, n_blocks: {}", n_blocks);
//   vkCmdDispatch(cmd_buf, n_blocks, 1u, 1u);
// }

// void Algorithm::set_push_constants(const void* data, uint32_t size) {
//   heterogeneous_push_constants_data_ = data;
//   heterogeneous_push_constants_size_ = size;
// }
