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
                     uint32_t threads_per_block,
                     const std::vector<float> &push_constants = {})
      : spirv_filename_(spirv_filename),
        device_(device),
        threads_per_block_(threads_per_block),
        usm_buffers_(buffers) {
    spdlog::info(
        "Algorithm::Algorithm() [{}]: Creating algorithm with {} buffers",
        spirv_filename_,
        buffers.size());

    if (!push_constants.empty()) {
      set_push_constants(push_constants);
    }

    create_shader_module();
    create_parameters();
    create_pipeline();
  }

  ~Algorithm() {
    spdlog::info("Algorithm::~Algorithm()");
    destroy();
  }

  void destroy();

  template <typename T>
  void set_push_constants(const std::vector<T> &push_constants) {
    const uint32_t memory_size = sizeof(decltype(push_constants.back()));
    const uint32_t size = push_constants.size();
    this->set_push_constants(push_constants.data(), size, memory_size);
  }

  void set_push_constants(const void *data,
                          uint32_t size,
                          uint32_t type_memory_size);

  void record_bind_core(VkCommandBuffer cmd_buf) const;
  void record_bind_push(VkCommandBuffer cmd_buf) const;
  void record_dispatch_tmp(VkCommandBuffer cmd_buf, uint32_t n) const;

 private:
  // L1
  void create_shader_module();

  void create_parameters() {
    create_descriptor_set_layout();
    create_descriptor_pool();
    allocate_descriptor_sets();
    update_descriptor_sets();
  }

  void create_pipeline() ;

  // L2
  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void allocate_descriptor_sets();
  void update_descriptor_sets();

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
