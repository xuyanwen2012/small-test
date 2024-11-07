#pragma once

#include <vector>

#include "base_engine.hpp"
#include "buffer.hpp"

class Engine : public BaseEngine {
 public:
  Engine() = default;
  ~Engine();

  [[nodiscard]] std::shared_ptr<Buffer> buffer(VkDeviceSize size) {
    auto buf = std::make_shared<Buffer>(this->get_device_ptr(), size);

    if (manage_resources_) {
      buffers_.push_back(buf);
    }
    return buf;
  }

  void destroy();

 private:
  // why weak_ptr? answer: to avoid circular reference
  std::vector<std::weak_ptr<Buffer>> buffers_;

  bool manage_resources_ = true;
};