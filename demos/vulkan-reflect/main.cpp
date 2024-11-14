#include <spirv_reflect.h>

#include <string_view>

#include "vulkan/base_engine.hpp"
#include "vulkan/shader_loader.hpp"

void reflect_shader(const std::string_view& filename) {
  const auto spirv_binary = load_shader_from_file(filename);

  SpvReflectShaderModule module;
  SpvReflectResult result = spvReflectCreateShaderModule(
      spirv_binary.size() * sizeof(uint32_t), spirv_binary.data(), &module);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to create SPIR-V reflection module.");
    return;
  }

  spdlog::info("Reflecting shader: {}", filename);
  spdlog::info("Entry point: {}", module.entry_point_name);

  // Get shader stage
  spdlog::info("Shader stage: {}",
               module.shader_stage == SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT
                   ? "Compute"
                   : "Other");

  // Query descriptor set count and layouts
  uint32_t descriptor_set_count = 0;
  result = spvReflectEnumerateDescriptorSets(
      &module, &descriptor_set_count, nullptr);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate descriptor sets.");
    return;
  }

  spdlog::info("Descriptor set count: {}", descriptor_set_count);

  // Query descriptor set layouts
  std::vector<SpvReflectDescriptorSet*> descriptor_sets(descriptor_set_count);
  result = spvReflectEnumerateDescriptorSets(
      &module, &descriptor_set_count, descriptor_sets.data());
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate descriptor sets.");
    return;
  }

  // Print detailed descriptor set and binding information
  for (uint32_t i = 0; i < descriptor_set_count; ++i) {
    const auto& set = descriptor_sets[i];
    spdlog::info("Descriptor set #{}", set->set);
    spdlog::info("  Binding count: {}", set->binding_count);

    for (uint32_t j = 0; j < set->binding_count; ++j) {
      const auto& binding = set->bindings[j];
      spdlog::info("    Binding #{}", binding->binding);
      spdlog::info("      Name: {}", binding->name);
      //   spdlog::info("      Type: {}", binding->descriptor_type);

      if (binding->type_description) {
        spdlog::info(
            "      Size: {} bytes",
            binding->type_description->traits.numeric.scalar.width / 8);
        if (binding->array.dims_count > 0) {
          spdlog::info("      Array dimensions: {}", binding->array.dims_count);
          for (uint32_t dim = 0; dim < binding->array.dims_count; dim++) {
            spdlog::info(
                "        Dimension {}: {}", dim, binding->array.dims[dim]);
          }
        }
      }
    }
  }

  // Query push constant blocks
  uint32_t push_constant_count = 0;
  result = spvReflectEnumeratePushConstantBlocks(
      &module, &push_constant_count, nullptr);
  if (result != SPV_REFLECT_RESULT_SUCCESS) {
    spdlog::error("Failed to enumerate push constant blocks.");
    return;
  }

  if (push_constant_count > 0) {
    std::vector<SpvReflectBlockVariable*> push_constants(push_constant_count);
    result = spvReflectEnumeratePushConstantBlocks(
        &module, &push_constant_count, push_constants.data());
    if (result != SPV_REFLECT_RESULT_SUCCESS) {
      spdlog::error("Failed to get push constant blocks.");
      return;
    }

    for (uint32_t i = 0; i < push_constant_count; ++i) {
      const auto& push_constant = push_constants[i];
      spdlog::info("Push constant block #{}", i);
      spdlog::info("  Name: {}", push_constant->name);
      spdlog::info("  Size: {} bytes", push_constant->size);
      spdlog::info("  Member count: {}", push_constant->member_count);

      for (uint32_t j = 0; j < push_constant->member_count; ++j) {
        const auto& member = push_constant->members[j];
        spdlog::info("    Member #{}", j);
        spdlog::info("      Name: {}", member.name);
        spdlog::info("      Offset: {} bytes", member.offset);
        spdlog::info("      Size: {} bytes", member.size);
      }
    }
  }

  // Get workgroup size if it's a compute shader
  if (module.shader_stage == SPV_REFLECT_SHADER_STAGE_COMPUTE_BIT) {
    const auto* entry_point =
        spvReflectGetEntryPoint(&module, module.entry_point_name);
    if (entry_point) {
      spdlog::info("Workgroup size: [{}, {}, {}]",
                   entry_point->local_size.x,
                   entry_point->local_size.y,
                   entry_point->local_size.z);
    }
  }

  // get the entry point name
  spdlog::info("Entry point name: {}", module.entry_point_name);
  

  spvReflectDestroyShaderModule(&module);
}

int main() {
  BaseEngine engine;

  //   for (const auto& entry : fs::directory_iterator(shader_base_path)) {
  //     if (entry.path().extension() == ".spv") {
  //       reflect_shader(entry.path().string());
  //     }
  //   }

  reflect_shader("test.spv");

  return 0;
}
