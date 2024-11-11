#pragma once

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

// Define the base path for shader files
// On Android, shaders are stored in "/data/local/tmp"
// On other platforms, use the current working directory
#ifdef __ANDROID__
constexpr std::string shader_base_path = "/data/local/tmp";
#else
// const std::string shader_base_path = fs::current_path().string();

// build/
// └── linux
//     └── x86_64
//         └── debug
//             ├── bench-cpu-pinned
//

// and then "(project root)/ppl/vulkan/shaders/compiled_shaders/"
const std::string shader_base_path = fs::current_path()
                                         .parent_path()
                                         .parent_path()
                                         .parent_path()
                                         .parent_path()
                                         .string() +
                                     "/ppl/vulkan/shaders/compiled_shaders/";

#endif

// SPIR-V shader files start with this magic number (0x07230203)
// Used to validate that a file is a valid SPIR-V shader
constexpr uint32_t SPIRV_MAGIC = 0x07230203;

/**
 * @brief Loads a SPIR-V shader from a file and returns its contents as a vector
 * of 32-bit integers
 *
 * @param filename The name of the shader file to load (relative to
 * shader_base_path)
 * @return std::vector<uint32_t> The shader contents as 32-bit integers
 * @throws std::runtime_error if:
 *   - The shader file is not found
 *   - The file cannot be opened
 *   - The file is too small (< 4 bytes)
 *   - There are errors reading the file
 *   - The file is not a valid SPIR-V shader (wrong magic number)
 */
[[nodiscard]] inline std::vector<uint32_t> load_shader_from_file(
    const std::string& filename) {
  const fs::path shader_path = fs::path(shader_base_path) / filename;

  spdlog::info("loading shader path: {}", shader_path.string());

  if (!fs::exists(shader_path)) {
    throw std::runtime_error("Shader file not found: " + shader_path.string());
  }

  std::ifstream file(shader_path, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + shader_path.string());
  }

  const size_t file_size = file.tellg();
  if (file_size < sizeof(uint32_t)) {
    throw std::runtime_error("Shader file too small: " + shader_path.string());
  }

  // Allocate buffer and read file contents
  // Size is divided by sizeof(uint32_t) because SPIR-V is stored as 32-bit
  // words
  std::vector<uint32_t> buffer(file_size / sizeof(uint32_t));
  file.seekg(0);  // Return to start of file
  file.read(reinterpret_cast<char*>(buffer.data()), file_size);

  file.close();

  if (file.fail()) {
    throw std::runtime_error("Failed to read file: " + shader_path.string());
  }

  // Validate that this is a SPIR-V shader by checking the magic number
  if (buffer.empty() || buffer[0] != SPIRV_MAGIC) {
    throw std::runtime_error(
        "Invalid SPIR-V shader file (wrong magic number): " +
        shader_path.string());
  }

  return buffer;
}
