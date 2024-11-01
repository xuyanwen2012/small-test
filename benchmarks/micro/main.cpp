#include <benchmark/benchmark.h>
#include <sched.h>
#include <unistd.h>

#include <iostream>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "utils.hpp"

// Function to pin thread to a specific core
void pin_thread_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);
  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    std::cerr << "Failed to pin thread to core " << core_id << std::endl;
  }
}

// 1. Monte Carlo Integration for Estimating Pi
double monte_carlo_pi(int points) {
  int inside_circle = 0;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dist(0.0, 1.0);

  for (int i = 0; i < points; ++i) {
    double x = dist(gen);
    double y = dist(gen);
    if (x * x + y * y <= 1.0) {
      inside_circle++;
    }
  }
  return (4.0 * inside_circle) / points;  // Estimate of pi
}

// 2. Matrix Multiplication Benchmark
void matrix_multiplication_benchmark(int size,
                                     std::vector<std::vector<double>>& matrixA,
                                     std::vector<std::vector<double>>& matrixB,
                                     std::vector<std::vector<double>>& result) {
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      result[i][j] = 0;
      for (int k = 0; k < size; ++k) {
        result[i][j] += matrixA[i][k] * matrixB[k][j];
      }
    }
  }
}

// 3. Irregular Graph Traversal
void graph_traversal_benchmark([[maybe_unused]] int nodes,
                               std::unordered_map<int, std::vector<int>>& graph,
                               int start_node,
                               int& visit_count) {
  std::unordered_set<int> visited;
  std::vector<int> stack = {start_node};
  visit_count = 0;

  while (!stack.empty()) {
    int node = stack.back();
    stack.pop_back();
    if (visited.find(node) == visited.end()) {
      visited.insert(node);
      ++visit_count;
      for (int neighbor : graph[node]) {
        if (visited.find(neighbor) == visited.end()) {
          stack.push_back(neighbor);
        }
      }
    }
  }
}

// Function to register Monte Carlo Pi benchmarks for all cores
void RegisterMonteCarloPiBenchmarks(
    std::span<const int> available_cores_to_pin) {
  for (int core_id : available_cores_to_pin) {
    auto mc_benchmark = [core_id](benchmark::State& state) {
      pin_thread_to_core(core_id);

      int points = state.range(0);
      for (auto _ : state) {
        monte_carlo_pi(points);
      }
    };
    benchmark::RegisterBenchmark(
        ("BM_monte_carlo_pi_Core" + std::to_string(core_id)).c_str(),
        mc_benchmark)
        ->Arg(100000)
        ->Unit(benchmark::kMillisecond);
  }
}

// Function to register Matrix Multiplication benchmarks for all cores
void RegisterMatrixMultiplicationBenchmarks(
    std::span<const int> available_cores_to_pin) {
  for (int core_id : available_cores_to_pin) {
    auto mm_benchmark = [core_id](benchmark::State& state) {
      pin_thread_to_core(core_id);
      int size = state.range(0);
      std::vector<std::vector<double>> matrixA(size,
                                               std::vector<double>(size, 1.5));
      std::vector<std::vector<double>> matrixB(size,
                                               std::vector<double>(size, 2.5));
      std::vector<std::vector<double>> result(size,
                                              std::vector<double>(size, 0.0));
      for (auto _ : state) {
        matrix_multiplication_benchmark(size, matrixA, matrixB, result);
      }
    };
    benchmark::RegisterBenchmark(
        ("BM_matrix_multiplication_Core" + std::to_string(core_id)).c_str(),
        mm_benchmark)
        ->Arg(200)
        ->Unit(benchmark::kMillisecond);
  }
}

// Function to register Graph Traversal benchmarks for all cores
void RegisterGraphTraversalBenchmarks(
    std::span<const int> available_cores_to_pin) {
  for (int core_id : available_cores_to_pin) {
    auto gt_benchmark = [core_id](benchmark::State& state) {
      pin_thread_to_core(core_id);
      int nodes = state.range(0);

      // Generate a random graph
      std::unordered_map<int, std::vector<int>> graph;
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(0, nodes - 1);
      for (int i = 0; i < nodes; ++i) {
        int connections = dis(gen) % 20 + 10;  // 10-30 connections per node
        for (int j = 0; j < connections; ++j) {
          graph[i].push_back(dis(gen));
        }
      }
      int visit_count = 0;
      for (auto _ : state) {
        graph_traversal_benchmark(nodes, graph, 0, visit_count);
      }
    };
    benchmark::RegisterBenchmark(
        ("BM_graph_traversal_Core" + std::to_string(core_id)).c_str(),
        gt_benchmark)
        ->Arg(20000)
        ->Unit(benchmark::kMillisecond);
  }
}

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);

  auto available_cores_to_pin = utils::get_available_cores();

  std::cout << "\tAvailable cores to pin: ";
  for (int core_id : available_cores_to_pin) {
    std::cout << core_id << " ";
  }
  std::cout << std::endl;

  // Register all benchmarks, grouping by benchmark type
  RegisterMonteCarloPiBenchmarks(available_cores_to_pin);
  RegisterMatrixMultiplicationBenchmarks(available_cores_to_pin);
  RegisterGraphTraversalBenchmarks(available_cores_to_pin);

  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}
