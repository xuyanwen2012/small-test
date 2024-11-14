#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "buffer.hpp"

class Algorithm final : public VulkanResource<VkShaderModule> {
 public:
  Algorithm() = delete;

  explicit Algorithm(std::shared_ptr<VkDevice> device_ptr,
                     std::string_view spirv_filename,
                     const std::vector<std::shared_ptr<Buffer>>& buffers,
                     const uint32_t push_constants_size)
      : VulkanResource<VkShaderModule>(std::move(device_ptr)),
        spirv_filename_(spirv_filename),
        usm_buffers_(buffers),
        push_constants_data_(push_constants_size),
        push_constants_size_(push_constants_size) {
    spdlog::debug(
        "Algorithm::Algorithm() [{}]: Creating algorithm with {} buffers",
        spirv_filename_,
        buffers.size());

    // load the shader
    create_shader_module();

    // parameters
    create_descriptor_set_layout();
    create_descriptor_pool();
    allocate_descriptor_sets();
    update_descriptor_sets();

    // pipeline itself
    create_pipeline();
  }

  ~Algorithm() override { destroy(); }

 protected:
  void destroy() override;

 public:
  template <typename T>
  void set_push_constants(const T& push_const_struct) {
    assert(push_constants_size_ == sizeof(T));
    std::memcpy(push_constants_data_.data(), &push_const_struct, sizeof(T));
  }

  // this method send the pipeline and descriptor set to the shader (basically
  // send the data in buffer to shader)
  void record_bind_core(VkCommandBuffer cmd_buf) const;

  // this method send the push constants to the shader
  void record_bind_push(VkCommandBuffer cmd_buf) const;

  // this method dispatch the kernel with the number of blocks provided.
  void record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
                                   uint32_t n_blocks) const;

 private:
  void create_shader_module();
  void create_pipeline();

  // L2
  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void allocate_descriptor_sets();
  void update_descriptor_sets();

  // Vulkan components
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  std::string spirv_filename_;

  std::vector<std::shared_ptr<Buffer>> usm_buffers_;
  std::vector<std::byte> push_constants_data_;
  uint32_t push_constants_size_ = 0;

  friend class Engine;
};
