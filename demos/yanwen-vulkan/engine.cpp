#include "engine.hpp"

Engine::~Engine() { destroy(); }

void Engine::destroy() {
  if (manage_resources_ && !buffers_.empty()) {
    spdlog::debug("ComputeEngine::destroy() explicitly freeing buffers");
    for (auto &weak_buffer : buffers_) {
      if (const auto buffer = weak_buffer.lock()) {
        buffer->destroy();
      }
    }
    buffers_.clear();
  }
}