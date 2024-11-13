#include <spirv_reflect.h>

#include <string_view>

#include "vulkan/base_engine.hpp"
#include "vulkan/shader_loader.hpp"

void reflect_shader(const std::string_view &filename) {
  const auto spirv_binary = load_shader_from_file(filename);

  SpvReflectShaderModule module;
  SpvReflectResult result = spvReflectCreateShaderModule(
      spirv_binary.size() * sizeof(uint32_t), spirv_binary.data(), &module);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to create SPIR-V reflection module.");
    return;
  }

  // Query descriptor set count and layouts
  uint32_t descriptor_set_count = 0;
  if (spvReflectEnumerateDescriptorSets(
          &module, &descriptor_set_count, nullptr) !=
      SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate descriptor sets.");
    return;
  }

  spdlog::info("Descriptor set count: {}", descriptor_set_count);

  // Query descriptor set layouts
  std::vector<SpvReflectDescriptorSet *> descriptor_sets(descriptor_set_count);
  if (spvReflectEnumerateDescriptorSets(
          &module, &descriptor_set_count, descriptor_sets.data()) !=
      SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate descriptor sets.");
    return;
  }

  for (uint32_t i = 0; i < descriptor_set_count; ++i) {
    spdlog::info("Descriptor set #{}", i);
    spdlog::info("  Binding count: {}", descriptor_sets[i]->binding_count);

    for (uint32_t j = 0; j < descriptor_sets[i]->binding_count; ++j) {
      spdlog::info("    Binding #{}", j);
    }
  }

  spvReflectDestroyShaderModule(&module);
}

int main() {
  BaseEngine engine;

  for (const auto &entry : fs::directory_iterator(shader_base_path)) {
    if (entry.path().extension() == ".spv") {
      reflect_shader(entry.path().string());
    }
  }

  return 0;
}
