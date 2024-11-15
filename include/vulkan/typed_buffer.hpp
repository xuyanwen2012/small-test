#pragma once

#include <cassert>
#include <ranges>
#include <span>

#include "buffer.hpp"

template <typename T>
class TypedBuffer final : public Buffer {
 public:
  // Iterator type aliases using span's iterators
  using iterator = typename std::span<T>::iterator;
  using const_iterator = typename std::span<const T>::iterator;

  TypedBuffer() = delete;

  explicit TypedBuffer(
      std::shared_ptr<VkDevice> device_ptr,
      const size_t count,
      const VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
      : Buffer(std::move(device_ptr), count * sizeof(T), usage),
        count_(count),
        mapped_typed_data_(as<T>()) {}

  // Core element access
  T& operator[](size_t index) {
    assert(index < count_);
    return mapped_typed_data_[index];
  }

  const T& operator[](size_t index) const {
    assert(index < count_);
    return mapped_typed_data_[index];
  }

  T& at(size_t index) {
    assert(index < count_);
    return mapped_typed_data_[index];
  }

  const T& at(size_t index) const {
    assert(index < count_);
    return mapped_typed_data_[index];
  }

  // Iterator support
  iterator begin() noexcept {
    return std::span{mapped_typed_data_, count_}.begin();
  }
  iterator end() noexcept {
    return std::span{mapped_typed_data_, count_}.end();
  }
  const_iterator begin() const noexcept {
    return std::span{mapped_typed_data_, count_}.begin();
  }
  const_iterator end() const noexcept {
    return std::span{mapped_typed_data_, count_}.end();
  }
  const_iterator cbegin() const noexcept { return begin(); }
  const_iterator cend() const noexcept { return end(); }

  // Size information
  [[nodiscard]] size_t size() const noexcept { return count_; }
  [[nodiscard]] size_t size_bytes() const noexcept {
    return count_ * sizeof(T);
  }

  // Data access
  T* data() noexcept { return mapped_typed_data_; }
  const T* data() const noexcept { return mapped_typed_data_; }

  // Span support for modern C++ range operations
  explicit operator std::span<T>() noexcept { return {mapped_typed_data_, count_}; }
  explicit operator std::span<const T>() const noexcept {
    return {mapped_typed_data_, count_};
  }

 private:
  size_t count_;
  T* mapped_typed_data_;
};

// Make TypedBuffer a range
template <typename T>
inline constexpr bool std::ranges::enable_borrowed_range<TypedBuffer<T>> = true;
