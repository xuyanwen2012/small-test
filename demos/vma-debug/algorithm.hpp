#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include "buffer.hpp"

class Algorithm : public VulkanResource<VkShaderModule> {
 public:
  Algorithm() = delete;

  explicit Algorithm(std::shared_ptr<VkDevice> device_ptr,
                     std::string_view spirv_filename,
                     const std::vector<std::shared_ptr<Buffer>> &buffers,
                     uint32_t threads_per_block,
                     const std::vector<float> &push_constants = {})
      : VulkanResource<VkShaderModule>(std::move(device_ptr)),
        spirv_filename_(spirv_filename),
        usm_buffers_(buffers),
        threads_per_block_(threads_per_block) {
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

  ~Algorithm() override {
    spdlog::info("Algorithm::~Algorithm()");
    destroy();
  }

 protected:
  void destroy() override;

 public:
  template <typename T>
  void set_push_constants(const std::vector<T> &push_constants) {
    const uint32_t memory_size = sizeof(decltype(push_constants.back()));
    const uint32_t size = push_constants.size();
    this->set_push_constants(push_constants.data(), size, memory_size);
  }

  void set_push_constants(const void *data,
                          uint32_t size,
                          uint32_t type_memory_size);

  // Let the cmd_buffer to bind my pipeline and descriptor set.
  void record_bind_core(VkCommandBuffer cmd_buf) const;

  // Let the cmd_buffer to bind my push constants.
  void record_bind_push(VkCommandBuffer cmd_buf) const;

  // Let the cmd_buffer to dispatch my kernel (compute shader).
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

  void create_pipeline();

  // L2
  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void allocate_descriptor_sets();
  void update_descriptor_sets();

  std::string spirv_filename_;

  // VkDevice device_ = VK_NULL_HANDLE;

  // Vulkan components
  // VkShaderModule shader_module_ = VK_NULL_HANDLE;
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  std::vector<std::shared_ptr<Buffer>> usm_buffers_;

  // Currently this assume the push constant is homogeneous type
  void *push_constants_data_ = nullptr;
  uint32_t push_constants_data_type_memory_size_ = 0;
  uint32_t push_constants_size_ = 0;

  /**
   * @brief In CUDA terms, this is the number threads per block. It is used to
   * describe work-items per work-group.
   */
  uint32_t threads_per_block_;
};
