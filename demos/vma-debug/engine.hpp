#pragma once

#include <vector>
#include <memory>

#include "base_engine.hpp"
#include "buffer.hpp"

class Engine final : public BaseEngine {
 public:
  Engine(bool manage_resources = true) : manage_resources_(manage_resources) {}

  ~Engine();

  [[nodiscard]] auto buffer(VkDeviceSize size) -> std::shared_ptr<Buffer> {
    auto buf = std::make_shared<Buffer>(this->get_device(), size);

    if (manage_resources_) {
      buffers_.push_back(buf);
    }
    return buf;
  }

  void destroy();

 private:
  std::vector<std::weak_ptr<Buffer>> buffers_;

  bool manage_resources_ = true;
};
