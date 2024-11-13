#pragma once

#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#include 

#include "buffer.hpp"

class Algorithm final : public VulkanResource<VkShaderModule> {
 public:
  Algorithm() = delete;

  // explicit Algorithm(std::shared_ptr<VkDevice> device_ptr,
  //                    std::string_view spirv_filename,
  //                    const std::vector<std::shared_ptr<Buffer>> &buffers,
  //                    uint32_t threads_per_block,
  //                    const std::byte *push_constants_data,
  //                    const uint32_t push_constants_size)
  //     : VulkanResource<VkShaderModule>(std::move(device_ptr)),
  //       spirv_filename_(spirv_filename),
  //       usm_buffers_(buffers),
  //       threads_per_block_(threads_per_block) {
  //   spdlog::debug(
  //       "Algorithm::Algorithm() [{}]: Creating algorithm with {} buffers",
  //       spirv_filename_,
  //       buffers.size());

  //   set_push_constants(push_constants_data, push_constants_size);

  //   create_shader_module();
  //   create_parameters();
  //   create_pipeline();
  // }

  explicit Algorithm(std::shared_ptr<VkDevice> device_ptr,
                     std::string_view spirv_filename,
                     const std::vector<std::shared_ptr<Buffer>> &buffers,
                     uint32_t threads_per_block,
                     const std::byte *push_constants_data,
                     const uint32_t push_constants_size)
      : VulkanResource<VkShaderModule>(std::move(device_ptr)),
        spirv_filename_(spirv_filename),
        usm_buffers_(buffers),
        threads_per_block_(threads_per_block) {
    spdlog::debug(
        "Algorithm::Algorithm() [{}]: Creating algorithm with {} buffers",
        spirv_filename_,
        buffers.size());

    set_push_constants(push_constants_data, push_constants_size);

    create_shader_module();
    create_parameters();
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

  // this method dispatch the kernel with the number of blocks based on the
  // input size. It will use as much blocks as needed to cover the input size.
  void record_dispatch_tmp(VkCommandBuffer cmd_buf, uint32_t n) const;

  // this method dispatch the kernel with the number of blocks provided.
  void record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
                                   uint32_t n_blocks) const;

 private:
  void set_push_constants(const std::byte *push_constants_data,
                          const uint32_t push_constants_size);
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

  // Vulkan components
  VkPipeline pipeline_ = VK_NULL_HANDLE;
  VkPipelineCache pipeline_cache_ = VK_NULL_HANDLE;
  VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
  VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
  VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

  std::vector<std::shared_ptr<Buffer>> usm_buffers_;

  // // Currently this assume the push constant is homogeneous type
  // void *push_constants_data_ = nullptr;
  // uint32_t push_constants_data_type_memory_size_ = 0;
  // uint32_t push_constants_size_ = 0;

  // this hetero genous push constants
  std::byte *heterogeneous_push_constants_data_ = nullptr;
  uint32_t heterogeneous_push_constants_size_ = 0;

  /**
   * @brief In CUDA terms, this is the number threads per block. It is used to
   * describe work-items per work-group.
   */
  uint32_t threads_per_block_;

  friend class Engine;
};
