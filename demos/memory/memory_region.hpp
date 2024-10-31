#pragma once

#include <memory>

#if defined(__CUDACC__)
#include <cuda_runtime.h>
#define DEVICE_BACKEND_CUDA
#else
#define DEVICE_BACKEND_CPU
#endif

// Base interface for MemoryRegion using concepts
template <typename T>
class MemoryRegionBase {
 public:
  virtual ~MemoryRegionBase() = default;
  virtual T& operator[](size_t index) = 0;
  virtual const T& operator[](size_t index) const = 0;
  virtual T* data() noexcept = 0;
  virtual const T* data() const noexcept = 0;
  [[nodiscard]] virtual size_t size() const noexcept = 0;
};

// Simplified CPU memory region implementation using raw pointer
template <typename T>
class CpuMemoryRegion : public MemoryRegionBase<T> {
  T* ptr_;
  size_t size_;

 public:
  explicit CpuMemoryRegion(size_t count) : size_(count) {
    ptr_ = new T[count]();  // Zero-initialize by default
  }

  ~CpuMemoryRegion() override { delete[] ptr_; }

  // Prevent copying
  CpuMemoryRegion(const CpuMemoryRegion&) = delete;
  CpuMemoryRegion& operator=(const CpuMemoryRegion&) = delete;

  // Allow moving
  CpuMemoryRegion(CpuMemoryRegion&& other) noexcept
      : ptr_(other.ptr_), size_(other.size_) {
    other.ptr_ = nullptr;
    other.size_ = 0;
  }

  CpuMemoryRegion& operator=(CpuMemoryRegion&& other) noexcept {
    if (this != &other) {
      delete[] ptr_;
      ptr_ = other.ptr_;
      size_ = other.size_;
      other.ptr_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  T& operator[](size_t index) override { return ptr_[index]; }
  const T& operator[](size_t index) const override { return ptr_[index]; }
  T* data() noexcept override { return ptr_; }
  const T* data() const noexcept override { return ptr_; }
  [[nodiscard]] size_t size() const noexcept override { return size_; }
};

#ifdef DEVICE_BACKEND_CUDA
template <typename T>
class CudaMemoryRegion : public MemoryRegionBase<T> {
  T* ptr_;
  size_t size_;

 public:
  explicit CudaMemoryRegion(size_t count) : size_(count) {
    if (cudaMallocManaged(&ptr_, size_ * sizeof(T)) != cudaSuccess) {
      throw std::runtime_error("CUDA memory allocation failed");
    }
  }

  ~CudaMemoryRegion() override {
    if (ptr_) cudaFree(ptr_);
  }

  // Prevent copying
  CudaMemoryRegion(const CudaMemoryRegion&) = delete;
  CudaMemoryRegion& operator=(const CudaMemoryRegion&) = delete;

  // Allow moving
  CudaMemoryRegion(CudaMemoryRegion&& other) noexcept
      : ptr_(other.ptr_), size_(other.size_) {
    other.ptr_ = nullptr;
    other.size_ = 0;
  }

  CudaMemoryRegion& operator=(CudaMemoryRegion&& other) noexcept {
    if (this != &other) {
      if (ptr_) cudaFree(ptr_);
      ptr_ = other.ptr_;
      size_ = other.size_;
      other.ptr_ = nullptr;
      other.size_ = 0;
    }
    return *this;
  }

  T& operator[](size_t index) override { return ptr_[index]; }
  const T& operator[](size_t index) const override { return ptr_[index]; }
  T* data() noexcept override { return ptr_; }
  const T* data() const noexcept override { return ptr_; }
  [[nodiscard]] size_t size() const noexcept override { return size_; }
};
#endif

// Simplified wrapper using std::unique_ptr
template <typename T>
class MemoryRegion {
  std::unique_ptr<MemoryRegionBase<T>> region_;

 public:
  explicit MemoryRegion(size_t count) {
#ifdef DEVICE_BACKEND_CUDA
    region_ = std::make_unique<CudaMemoryRegion<T>>(count);
#else
    region_ = std::make_unique<CpuMemoryRegion<T>>(count);
#endif
  }

  T& operator[](size_t index) { return (*region_)[index]; }
  const T& operator[](size_t index) const { return (*region_)[index]; }
  T* data() noexcept { return region_->data(); }
  const T* data() const noexcept { return region_->data(); }
  [[nodiscard]] size_t size() const noexcept { return region_->size(); }
};
