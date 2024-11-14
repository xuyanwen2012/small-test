#include "vulkan/engine.hpp"

#include <spdlog/spdlog.h>

Engine::~Engine() { destroy(); }

void Engine::destroy() const {
  spdlog::debug("Engine::destroy()");

  if (manage_resources_ && !buffers_.empty()) {
    spdlog::debug("Engine::destroy() explicitly freeing buffers");
    for (auto &weak_buffer : buffers_) {
      if (const auto buffer = weak_buffer.lock()) {
        buffer->destroy();
      }
    }
  }

  if (manage_resources_ && !algorithms_.empty()) {
    spdlog::debug("Engine::destroy() explicitly freeing algorithms");
    for (auto &weak_algorithm : algorithms_) {
      if (const auto algorithm = weak_algorithm.lock()) {
        algorithm->destroy();
      }
    }
  }

  if (manage_resources_ && !sequences_.empty()) {
    spdlog::debug("Engine::destroy() explicitly freeing sequences");
    for (auto &weak_sequence : sequences_) {
      if (const auto sequence = weak_sequence.lock()) {
        sequence->destroy();
      }
    }
  }
}
