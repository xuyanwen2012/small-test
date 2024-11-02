#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

struct PhoneSpecs {
  const std::string alias;
  const int core_count;
  const std::vector<int> valid_cores;
  const std::vector<int> small_cores;
  const std::vector<int> mid_cores;
  const std::vector<int> big_cores;
  const double small_core_freq;  // GHz
  const double mid_core_freq;    // GHz
  const double big_core_freq;    // GHz
};

inline const std::unordered_map<std::string, const PhoneSpecs> PHONE_SPECS{
    {"3A021JEHN02756",
     {.alias = "Pixel_7a",
      .core_count = 8,
      .valid_cores = {0, 1, 2, 3, 4, 5, 6, 7},
      .small_cores = {0, 1, 2, 3},
      .mid_cores = {4, 5},
      .big_cores = {6, 7},
      .small_core_freq = 1.803,
      .mid_core_freq = 2.348,
      .big_core_freq = 2.85}},
    {"9b034f1b",
     {
         .alias = "OnePlus",
         .core_count = 8,
         .valid_cores = {0, 1, 2, 5},
         .small_cores = {0, 1, 2},
         .mid_cores = {3, 4, 5, 6},
         .big_cores = {7},
         .small_core_freq = 0.0,  // Unknown
         .mid_core_freq = 0.0,    // Unknown
         .big_core_freq = 0.0     // Unknown
     }},
    {"RFCT80DAADN",
     {.alias = "Samsung-new",
      .core_count = 8,
      .valid_cores = {0, 1, 2, 3, 4, 5, 6, 7},  // Updated based on output
      .small_cores = {0, 1, 2, 3},
      .mid_cores = {4, 5, 6},
      .big_cores = {7},
      .small_core_freq = 1.824,
      .mid_core_freq = 2.515,
      .big_core_freq = 2.803}},
    {"ZY22FLDDK7",
     {
         .alias = "Motorola",
         .core_count = 8,
         .valid_cores = {0, 1, 2, 3, 4, 5, 6, 7},
         .small_cores = {4, 5, 6, 7},
         .mid_cores = {},
         .big_cores = {0, 1, 2, 3},
         .small_core_freq = 0.0,  // Unknown
         .mid_core_freq = 0.0,    // Unknown
         .big_core_freq = 0.0     // Unknown
     }},
    {"ce0717178d7758b00b7e",
     {.alias = "Samsung-old",
      .core_count = 8,
      .valid_cores = {0, 1, 2, 3, 4, 5},  // Updated based on output
      .small_cores = {0, 1, 2, 3},
      .mid_cores = {},
      .big_cores = {4, 5, 6, 7},
      .small_core_freq = 1.9008,
      .mid_core_freq = 0.0,  // No mid cores
      .big_core_freq = 2.3616}},
    {"jetson",
     {
         .alias = "Jetson Orin",
         .core_count = 6,
         .valid_cores = {0, 1, 2, 3, 4, 5},
         .small_cores = {0, 1, 2, 3, 4, 5},
         .mid_cores = {},
         .big_cores = {},
         .small_core_freq = 0.0,  // Unknown
         .mid_core_freq = 0.0,    // Unknown
         .big_core_freq = 0.0     // Unknown
     }}};

inline std::optional<const PhoneSpecs*> get_phone_specs(
    const std::string& device_id) {
  auto it = PHONE_SPECS.find(device_id);
  if (it != PHONE_SPECS.end()) {
    return &it->second;
  }
  return std::nullopt;
}
