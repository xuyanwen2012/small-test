#include <iostream>

#include "vulkan/app_params.hpp"
#include "vulkan/naive_pipe.hpp"

#define BUFFER_ELEMENTS 1920 * 1080
#define MAX_BLOCKS 128
#define ITERATIONS 1

int main() {
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize volk!" << std::endl;
    return EXIT_FAILURE;
  }
  const auto n_blocks = 1;

  AppParams app_params;
  app_params.n = BUFFER_ELEMENTS;
  app_params.min_coord = 0.0f;
  app_params.max_coord = 1.0f;
  app_params.seed = 114514;
  app_params.n_threads = 4;
  app_params.n_blocks = n_blocks;

  Pipe pipe = Pipe(app_params);
  pipe.allocate();

  pipe.init(n_blocks, 0);

  pipe.morton(n_blocks, 0);

  std::cout << "done" << std::endl;

  return 0;
}
