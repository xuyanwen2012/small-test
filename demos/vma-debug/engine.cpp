#include "engine.hpp"

#include <spdlog/spdlog.h>

Engine::~Engine() { destroy(); }

void Engine::destroy() {
  


  // if (manage_resources_ && !buffers_.empty()) {
  //   spdlog::debug("Engine::destroy() explicitly freeing buffers");
  //   for (auto &weak_buffer : buffers_) {
  //     if (const auto buffer = weak_buffer.lock()) {
  //       buffer->destroy();
  //     }
  //   }
  // }
}


