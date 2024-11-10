#pragma once

#include "algorithm.hpp"
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

  /**
   * @brief Record the commands of an Algorithm. It will bind pipeline, push
   * constants, and dispatch.
   *
   * @param algo Algorithm to be recorded.
   * @param n Number of elements to be processed.
   */
  void simple_record_commands(const Algorithm &algo, const uint32_t n) const {
    cmd_begin();
    algo.record_bind_core(command_buffer_);
    algo.record_bind_push(command_buffer_);
    algo.record_dispatch_tmp(command_buffer_, n);
    cmd_end();
  }

  /**
   * @brief Once all commands are recorded, you can launch the kernel. It will
   * submit all the commands to the GPU. It is asynchronous, so you can do other
   * CPU tasks while the GPU is processing.
   */
  void launch_kernel_async();

  /**
   * @brief Wait for the GPU to finish all the commands. This is like
   * "cudaDeviceSynchronize()"
   */
  void sync() const;

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
