#define VK_NO_PROTOTYPES
#include <cstring>
#include <iostream>
#include <vector>

#include "volk.h"

bool checkValidationLayerSupport() {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
  for (const auto& layer : availableLayers) {
    if (strcmp(layer.layerName, validationLayerName) == 0) {
      return true;
    }
  }
  return false;
}

int main() {
  if (volkInitialize() != VK_SUCCESS) {
    std::cerr << "Failed to initialize Volk.\n";
    return -1;
  }

  if (checkValidationLayerSupport()) {
    std::cout << "Validation layer is available.\n";
  } else {
    std::cout << "Validation layer is not available.\n";
  }
  return 0;
}
