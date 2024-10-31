#pragma once

#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sched.h>
#include <span>
#include <string_view>
#include <thread>
#include <vector>

/**
 * @namespace utils
 * @brief Utility functions for CPU management and system information
 *
 * This namespace contains functions for CPU core management, frequency
 * monitoring, and process affinity control. It provides tools for both Android
 * and Linux systems to query and manipulate CPU-related settings.
 */
namespace utils {

using namespace std::chrono_literals;

/**
 * @brief Performs a busy-wait for the specified duration with high precision
 *
 * @details This function implements a busy-wait loop that provides
 * high-precision timing. On Android platforms, it includes a yield operation to
 * prevent excessive CPU usage while maintaining timing accuracy.
 *
 * @tparam Rep The arithmetic type representing the number of ticks
 * @tparam Period The ratio type representing the tick period
 * @param duration The time duration to wait
 */
template <typename Rep, typename Period>
inline void busy_wait_for(std::chrono::duration<Rep, Period> duration) {
  const auto start = std::chrono::steady_clock::now();
  while (std::chrono::steady_clock::now() - start < duration) {
#if defined(__ANDROID__)
    sched_yield();
#endif
  }
}

/**
 * @brief Retrieves a list of CPU cores available to the current process
 *
 * @details Uses the process CPU affinity mask to determine which cores are
 * available. The function handles error cases and provides appropriate logging.
 *
 * @return std::vector<int> Vector containing available CPU core indices
 * @throws std::system_error If unable to get CPU affinity
 */
[[nodiscard]]
inline std::vector<int> get_available_cores() {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  if (sched_getaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
    throw std::system_error(errno, std::system_category(),
                            "Failed to get CPU affinity");
  }

  std::vector<int> available_cores;
  available_cores.reserve(CPU_SETSIZE);

  for (int core = 0; core < CPU_SETSIZE; ++core) {
    if (CPU_ISSET(core, &cpuset)) {
      available_cores.push_back(core);
    }
  }

  return available_cores;
}

/**
 * @brief Checks if one vector of core IDs is a subset of another
 *
 * @details Uses C++20 ranges to efficiently verify subset relationships between
 * core ID collections. This is particularly useful for validating core
 * assignments.
 *
 * @param subset Vector of core IDs to check
 * @param superset Vector of core IDs that should contain subset
 * @return bool True if subset is contained within superset
 */
[[nodiscard]]
constexpr bool is_subset(std::span<const int> subset,
                         std::span<const int> superset) {
  return std::ranges::all_of(subset, [&superset](const int core) {
    return std::ranges::find(superset, core) != superset.end();
  });
}

/**
 * @brief Sets the CPU affinity for the current process
 *
 * @details Attempts to bind the current process to run exclusively on the
 * specified CPU core. Provides error handling and logging.
 *
 * @param core_id The target CPU core ID
 * @throws std::system_error If setting affinity fails
 */
inline void set_cpu_affinity(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) != 0) {
    throw std::system_error(errno, std::system_category(),
                            "Failed to set CPU affinity to core " +
                                std::to_string(core_id));
  }
}

/**
 * @brief Represents CPU frequency information
 */
struct CoreFrequency {
  int core_id;
  double frequency_ghz;

  // Comparison operators for sorting
  auto operator<=>(const CoreFrequency &) const = default;
};

/**
 * @brief Retrieves the frequencies of all available CPU cores
 *
 * @details Implements a platform-specific approach to reading CPU frequencies:
 * - On Android: Reads from sysfs (cpuinfo_max_freq)
 * - On Linux: Falls back to /proc/cpuinfo if sysfs is unavailable
 *
 * @return std::vector<CoreFrequency> Vector of core frequencies
 */
[[nodiscard]]
inline std::vector<CoreFrequency> get_core_frequencies() {
  std::vector<CoreFrequency> frequencies;

#if defined(__ANDROID__)
  constexpr std::string_view freq_path =
      "/sys/devices/system/cpu/cpu%d/cpufreq/cpuinfo_max_freq";

  for (int core_index = 0;; ++core_index) {
    char path[256];
    snprintf(path, sizeof(path), freq_path.data(), core_index);

    std::ifstream file(path);
    if (!file)
      break;

    int freq_khz;
    if (file >> freq_khz) {
      frequencies.push_back(
          {core_index, static_cast<double>(freq_khz) / 1'000'000.0});
    }
  }
#else
  std::ifstream cpuinfo("/proc/cpuinfo");
  if (cpuinfo) {
    std::string line;
    int current_core = -1;

    while (std::getline(cpuinfo, line)) {
      if (line.starts_with("processor")) {
        current_core = std::stoi(line.substr(line.find(':') + 1));
      } else if (line.starts_with("cpu MHz")) {
        auto pos = line.find(':');
        if (pos != std::string::npos) {
          double freq_mhz = std::stod(line.substr(pos + 1));
          frequencies.push_back({current_core, freq_mhz / 1000.0});
        }
      }
    }
  }
#endif

  std::ranges::sort(frequencies);
  return frequencies;
}

/**
 * @brief Displays detailed system core information
 *
 * @details Prints comprehensive information about the system's CPU cores
 * including:
 * - Total number of cores
 * - Core frequencies
 * - Available cores for pinning
 *
 * @throws std::runtime_error If no cores are available
 */
inline void display_core_info() {
  std::cout << "System Core Information:\n"
            << "----------------------\n"
            << "Total logical cores: " << std::thread::hardware_concurrency()
            << '\n';

  const auto frequencies = get_core_frequencies();
  if (!frequencies.empty()) {
    std::cout << "\nCore Frequencies:\n";
    for (const auto &[core, freq] : frequencies) {
      std::cout << "Core " << core << ": " << freq << " GHz\n";
    }
  } else {
    std::cout << "Unable to read core frequencies\n";
  }

  const auto available_cores = get_available_cores();
  if (available_cores.empty()) {
    throw std::runtime_error("No CPU cores available for the current process");
  }

  std::cout << "\nAvailable cores for pinning: ";
  std::ranges::copy(available_cores,
                    std::ostream_iterator<int>(std::cout, " "));
  std::cout << '\n';
}

/**
 * @brief Validates if the specified cores are available for use
 *
 * @param cores Vector of core IDs to validate
 * @return bool True if all specified cores are available
 * @throws std::runtime_error If core validation fails
 */
[[nodiscard]]
inline bool validate_cores(std::span<const int> cores) {
  const auto available_cores = get_available_cores();

  if (!is_subset(cores, available_cores)) {
    throw std::runtime_error("Specified cores are not available for use");
  }

  return true;
}

} // namespace utils
