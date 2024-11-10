#pragma once

#include "base_engine.hpp"

class Sequence {
 public:
  explicit Sequence(VkDevice device,
                    VkQueue queue,
                    uint32_t compute_queue_index)
      : device_(device),
        queue_(queue),
        compute_queue_index_(compute_queue_index) {
    create_sync_objects();
    create_command_pool();
    create_command_buffer();
  }

  ~Sequence() { destroy(); }
  void destroy();

  // ---------------------------------------------------------------------------
  //             Operations you can do w/ the command buffer
  // ---------------------------------------------------------------------------

  void cmd_begin() const;
  void cmd_end() const;

 private:
  void create_sync_objects();
  void create_command_pool();
  void create_command_buffer();

  // ref
  VkDevice device_ = VK_NULL_HANDLE;
  VkQueue queue_ = VK_NULL_HANDLE;
  uint32_t compute_queue_index_ = 0;

  // owned
  VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;
};
