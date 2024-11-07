#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "buffer.hpp"

class Algorithm {
 public:
  Algorithm() = delete;

  explicit Algorithm(VkDevice device,
                     std::string_view spirv_filename,
                     const std::vector<std::shared_ptr<Buffer>> &buffers,
                     uint32_t threads_per_block)
      : spirv_filename_(spirv_filename),
        device_(device),
        threads_per_block_(threads_per_block),
        usm_buffers_(buffers) {
    spdlog::info("Algorithm::Algorithm() [{}]: Creating algorithm with {} buffers",
                 spirv_filename_,
                 buffers.size());

    create_parameters();
    create_pipeline();
    create_shader_module();
  }

  ~Algorithm() {
    spdlog::info("Algorithm::~Algorithm()");
    destroy();
  }

  void destroy();

  void record_bind_core(VkCommandBuffer cmd_buf) const;
  void record_bind_push(VkCommandBuffer cmd_buf) const;
  void record_dispatch_tmp(VkCommandBuffer cmd_buf, uint32_t n) const;

 private:
  void create_parameters();
  void create_pipeline();
  void create_shader_module();

  std::string spirv_filename_;

  VkDevice device_ = VK_NULL_HANDLE;

  // Vulkan components
  VkShaderModule shader_module_ = VK_NULL_HANDLE;
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  uint32_t threads_per_block_;

  std::vector<std::shared_ptr<Buffer>> usm_buffers_;

  // Currently this assume the push constant is homogeneous type
  void *push_constants_data_ = nullptr;
  uint32_t push_constants_data_type_memory_size_ = 0;
  uint32_t push_constants_size_ = 0;
};
