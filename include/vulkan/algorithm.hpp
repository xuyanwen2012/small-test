#pragma once

#include <spdlog/spdlog.h>
#include <spirv_reflect.h>

#include <string_view>
#include <vector>
#include <unordered_map>

#include "buffer.hpp"
#include "vulkan/vulkan_resource.hpp"

class Algorithm : public VulkanResource<VkShaderModule> {
 public:
  Algorithm() = delete;

  explicit Algorithm(std::shared_ptr<VkDevice> device_ptr,
                     std::string_view spirv_filename)
      : VulkanResource<VkShaderModule>(std::move(device_ptr)),
        spirv_filename_(spirv_filename) {
    spdlog::debug(
        "Algorithm::Algorithm() [{}]: Creating algorithm, about to reflect "
        ".spv file",
        spirv_filename_);

    create_shader_module();

    create_descriptor_set_layout();
    create_descriptor_pool();

    allocate_descriptor_sets();
    update_descriptor_sets();

    create_pipeline();
  }

  ~Algorithm() override { destroy(); }

 protected:
  void destroy() override;

 public:
  // this method send the pipeline and descriptor set to the shader (basically
  // send the data in buffer to shader)
  void record_bind_core(VkCommandBuffer cmd_buf) const;

  // this method send the push constants to the shader
  void record_bind_push(VkCommandBuffer cmd_buf) const;

  // this method dispatch the kernel with the number of blocks provided.
  void record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
                                   uint32_t n_blocks) const;

  void set_push_constants(const void* data, uint32_t size);

  // Add methods to register resources
  void register_storage_buffer(uint32_t binding, std::shared_ptr<Buffer> buffer);

 private:
  // void set_push_constants(const std::byte *push_constants_data,
  //                         const uint32_t push_constants_size);

  void create_shader_module();

  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void allocate_descriptor_sets();
  void update_descriptor_sets();

  void create_pipeline();

  // Vulkan components
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  // filename of the .spv file
  std::string spirv_filename_;
  SpvReflectShaderModule reflect_module_;
  
  // // parameters passed to the shader
  // std::vector<std::shared_ptr<Buffer>> usm_buffers_;
  std::vector<std::byte> push_constants_data_;

  // Add storage for resources
  std::unordered_map<uint32_t, std::shared_ptr<Buffer>> storage_buffers_;

  friend class Engine;
};
