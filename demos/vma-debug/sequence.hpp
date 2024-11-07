#pragma once

#include "base_engine.hpp"

class Sequence {
 public:
  explicit Sequence(BaseEngine &engine) {
    create_sync_objects();
    create_command_pool();
    create_command_buffer();
  }

  ~Sequence();
   void destroy() ;

  // ---------------------------------------------------------------------------
  //             Operations you can do w/ the command buffer
  // ---------------------------------------------------------------------------

  void cmd_begin() const;
  void cmd_end() const;


 private:
  void create_sync_objects();
  void create_command_pool();
  void create_command_buffer();

  VkDevice device_ = VK_NULL_HANDLE;
  
  VkQueue queue_ = VK_NULL_HANDLE;
  VkCommandPool command_pool_ = VK_NULL_HANDLE;
  VkFence fence_ = VK_NULL_HANDLE;
};
