#pragma once

#include <memory>
#include <vector>

#include "algorithm.hpp"
#include "base_engine.hpp"
#include "buffer.hpp"
#include "sequence.hpp"

class Engine final : public BaseEngine {
 public:
  Engine(bool manage_resources = true) : manage_resources_(manage_resources) {}

  ~Engine();

  [[nodiscard]] auto buffer(VkDeviceSize size) -> std::shared_ptr<Buffer> {
    auto buf = std::make_shared<Buffer>(this->get_device_ptr(), size);

    if (manage_resources_) {
      buffers_.push_back(buf);
    }
    return buf;
  }

  template <typename T>
  [[nodiscard]] auto algorithm(
      const std::string &spirv_filename,
      const std::vector<std::shared_ptr<Buffer>> &buffers,
      uint32_t threads_per_block,
      const std::vector<T> &push_constants = {})
      -> std::shared_ptr<Algorithm> {
    auto algo = std::make_shared<Algorithm>(this->get_device_ptr(),
                                            spirv_filename,
                                            buffers,
                                            threads_per_block,
                                            push_constants);

    if (manage_resources_) {
      algorithms_.push_back(algo);
    }
    return algo;
  }

  [[nodiscard]] auto sequence() -> std::shared_ptr<Sequence> {
    auto seq = std::make_shared<Sequence>(this->get_device_ptr(),
                                          this->get_queue(),
                                          this->get_compute_queue_index());

    if (manage_resources_) {
      sequences_.push_back(seq);
    }
    return seq;
  }

  void destroy();

 private:
  std::vector<std::weak_ptr<Buffer>> buffers_;
  std::vector<std::weak_ptr<Algorithm>> algorithms_;
  std::vector<std::weak_ptr<Sequence>> sequences_;

  bool manage_resources_ = true;
};
