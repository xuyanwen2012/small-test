#pragma once

#include <cstddef>
#include <memory>
#include <cstdlib>

// Include CUDA header if CUDA is enabled
#ifdef ENABLE_CUDA
#include <cuda_runtime.h>
#endif

enum class Backend {
  CPU,
  CUDA
};

class MemoryRegion {
public:
  void* ptr = nullptr;
  size_t size = 0;
  Backend backend;
  
  MemoryRegion() = default;
  
  // Constructor
  MemoryRegion(void* ptr, size_t size, Backend backend) 
    : ptr(ptr), size(size), backend(backend) {}
    
  // Move constructor
  MemoryRegion(MemoryRegion&& other) noexcept 
    : ptr(other.ptr), size(other.size), backend(other.backend) {
    other.ptr = nullptr;
    other.size = 0;
  }
  
  // Move assignment
  MemoryRegion& operator=(MemoryRegion&& other) noexcept {
    if (this != &other) {
      ptr = other.ptr;
      size = other.size;
      backend = other.backend;
      other.ptr = nullptr;
      other.size = 0;
    }
    return *this;
  }
  
  // Disable copying
  MemoryRegion(const MemoryRegion&) = delete;
  MemoryRegion& operator=(const MemoryRegion&) = delete;
  
  // Templated accessor
  template<typename T>
  T* as() { return static_cast<T*>(ptr); }
  
  template<typename T>
  const T* as() const { return static_cast<const T*>(ptr); }
};

class MemoryAllocator {
public:
  virtual ~MemoryAllocator() = default;
  virtual MemoryRegion allocate(size_t size) = 0;
  virtual void deallocate(MemoryRegion& region) = 0;
  
  static std::unique_ptr<MemoryAllocator> create(Backend backend);
}; 