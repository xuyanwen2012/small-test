#include "memory_region.hpp"

#include <stdexcept>

namespace {

class CPUAllocator final : public MemoryAllocator {
 public:
  MemoryRegion allocate(size_t size) override {
    void* ptr = std::malloc(size);
    if (!ptr) {
      throw std::bad_alloc();
    }
    return MemoryRegion(ptr, size, Backend::CPU);
  }

  void deallocate(MemoryRegion& region) override {
    if (region.ptr) {
      std::free(region.ptr);
      region.ptr = nullptr;
      region.size = 0;
    }
  }
};

#ifdef ENABLE_CUDA
class CUDAAllocator final : public MemoryAllocator {
 public:
  MemoryRegion allocate(size_t size) override {
    void* ptr = nullptr;
    cudaError_t err = cudaMallocManaged(&ptr, size);
    if (err != cudaSuccess || !ptr) {
      throw std::runtime_error("CUDA allocation failed");
    }
    return MemoryRegion(ptr, size, Backend::CUDA);
  }

  void deallocate(MemoryRegion& region) override {
    if (region.ptr) {
      cudaFree(region.ptr);
      region.ptr = nullptr;
      region.size = 0;
    }
  }
};
#endif

}  // anonymous namespace

std::unique_ptr<MemoryAllocator> MemoryAllocator::create(Backend backend) {
  switch (backend) {
    case Backend::CPU:
      return std::make_unique<CPUAllocator>();

#ifdef ENABLE_CUDA
    case Backend::CUDA:
      return std::make_unique<CUDAAllocator>();
#endif

    default:
      throw std::runtime_error("Unsupported backend");
  }
}