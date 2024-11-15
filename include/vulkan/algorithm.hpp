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
                     uint32_t push_constants_size);

  ~Algorithm() override { Algorithm::destroy(); }

 protected:
  void destroy() override;

public:
  [[nodiscard]] bool has_push_constants() const {
    return push_constants_size_ > 0;
  }

  template <typename T>
  void set_push_constants(const T& push_const_struct) {
    assert(push_constants_size_ == sizeof(T));
    std::memcpy(push_constants_data_.data(), &push_const_struct, sizeof(T));
  }

  void update_descriptor_sets_with_buffers(
      const std::vector<std::shared_ptr<Buffer>>& buffers) const;
  void update_descriptor_sets() const;

  void record_bind_core(VkCommandBuffer cmd_buf) const;
  void record_bind_push(VkCommandBuffer cmd_buf) const;
  static void record_dispatch_with_blocks(VkCommandBuffer cmd_buf,
                                          uint32_t n_blocks);

 private:

  void create_shader_module();
  void create_pipeline();
  void create_descriptor_set_layout();
  void create_descriptor_pool();
  void allocate_descriptor_sets();

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
