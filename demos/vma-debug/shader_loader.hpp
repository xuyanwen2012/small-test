#pragma once

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

// if on android, base shader path is "/data/local/tmp"
#ifdef __ANDROID__
constexpr std::string shader_base_path = "/data/local/tmp";
#else
const std::string shader_base_path = fs::current_path().string();
#endif

// SPIR-V magic number (0x07230203)
constexpr uint32_t SPIRV_MAGIC = 0x07230203;

[[nodiscard]] inline std::vector<uint32_t> load_shader_from_file(
    const std::string& filename) {
  const fs::path shader_path = fs::path(shader_base_path) / filename;

  spdlog::info("loading shader path: {}", shader_path.string());

  if (!fs::exists(shader_path)) {
    throw std::runtime_error("Shader file not found: " + shader_path.string());
  }

  // Open the shader file using the full path
  std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + shader_path.string());
  }

  const size_t file_size = file.tellg();
  if (file_size < sizeof(uint32_t)) {
    throw std::runtime_error("Shader file too small: " + shader_path.string());
  }

  // Read the file into a vector
  std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
  file.seekg(0);
  file.read(reinterpret_cast<char*>(buffer.data()), file_size);

  file.close();

  if (file.fail()) {
    throw std::runtime_error("Failed to read file: " + shader_path.string());
  }

  // Validate SPIR-V magic number
  if (buffer.empty() || buffer[0] != SPIRV_MAGIC) {
    throw std::runtime_error(
        "Invalid SPIR-V shader file (wrong magic number): " +
        shader_path.string());
  }

  return buffer;
}

// [[nodiscard]] inline std::vector<uint32_t> load_shader_from_file(
//     const std::string& filename) {
//   const fs::path shader_path = fs::path(shader_base_path) / filename;

//   spdlog::info("loading shader path: {}", shader_path.string());

//   if (!fs::exists(shader_path)) {
//     throw std::runtime_error("Shader file not found: " +
//     shader_path.string());
//   }

//   // Open the shader file using the full path
//   std::ifstream file(shader_path, std::ios::ate | std::ios::binary);
//   if (!file.is_open()) {
//     throw std::runtime_error("Failed to open file: " + shader_path.string());
//   }

//   // Get the file size in bytes
//   const size_t file_size = file.tellg();
//   file.seekg(0);

//   if (file_size < sizeof(uint32_t)) {
//     throw std::runtime_error("Shader file too small: " +
//     shader_path.string());
//   }

//   // Check if padding is needed
//   size_t word_count = file_size / sizeof(uint32_t);
//   bool needs_padding = (file_size % sizeof(uint32_t)) != 0;
//   if (needs_padding) {
//     word_count += 1;  // Add an extra 4-byte word for padding
//   }

//   // Read the file into a vector of uint32_t
//   std::vector<uint32_t> buffer(
//       word_count, 0);  // Initialize with zeros if padding is needed
//   file.read(reinterpret_cast<char*>(buffer.data()), file_size);
//   file.close();

//   if (file.fail()) {
//     throw std::runtime_error("Failed to read file: " + shader_path.string());
//   }

//   // Validate SPIR-V magic number
//   if (buffer.empty() || buffer[0] != SPIRV_MAGIC) {
//     throw std::runtime_error(
//         "Invalid SPIR-V shader file (wrong magic number): " +
//         shader_path.string());
//   }

//   return buffer;
// }
