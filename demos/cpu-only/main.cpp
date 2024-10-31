#include <iostream>
#include <memory>
#include <random>
#include <vector>
#include <chrono>
#include <iomanip>

#include "core/thread_pool.hpp"
#include "shared/structures.h"
#include "third-party/CLI11.hpp"
#include "utils.hpp"
#include "host_dispatcher.hpp"

namespace {
    struct Config {
        static constexpr int DEFAULT_N = 640 * 480;  // ~300k
        static constexpr float DEFAULT_MIN_COORD = 0.0f;
        static constexpr float DEFAULT_RANGE = 1024.0f;
        static constexpr unsigned DEFAULT_SEED = 114514;
        static constexpr int DEFAULT_ITERATIONS = 30;
    };

    void gen_data(const std::shared_ptr<Pipe>& p, unsigned seed) {
        std::mt19937 gen(seed);
        std::uniform_real_distribution dis(p->min_coord, p->min_coord + p->range);
        std::generate_n(p->u_points, p->n_input(), [&dis, &gen] {
            return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
        });
    }

    void print_points(const std::shared_ptr<const Pipe>& p, size_t count) {
        std::cout << "\nFirst " << count << " points:\n";
        for (size_t i = 0; i < count && i < p->n_input(); ++i) {
            const auto& point = p->u_points[i];
            std::cout << std::fixed << std::setprecision(2)
                     << "(" << point.x << ", " << point.y << ", " << point.z << ")\n";
        }
    }

    void print_morton_codes(const std::shared_ptr<const Pipe>& p, size_t count) {
        std::cout << "\nFirst " << count << " morton codes:\n";
        for (size_t i = 0; i < count && i < p->n_input(); ++i) {
            std::cout << "0x" << std::hex << p->u_morton[i] << std::dec << "\n";
        }
    }
}

int main(int argc, char** argv) {
    CLI::App app{"CPU Morton code computation demo"};

    // Command line parameters
    std::vector<int> small_cores;
    int num_threads;
    int problem_size;
    int iterations;
    
    app.add_option("-s,--small_cores", small_cores, "Small cores to use")
        ->default_val(std::vector<int>{0, 1, 2, 3})
        ->check(CLI::Range(0, 8));
    
    app.add_option("-t,--threads", num_threads, "Number of threads")
        ->default_val(4)
        ->check(CLI::Range(1, 8));
        
    app.add_option("-n,--size", problem_size, "Problem size")
        ->default_val(Config::DEFAULT_N);
        
    app.add_option("-i,--iterations", iterations, "Number of iterations")
        ->default_val(Config::DEFAULT_ITERATIONS)
        ->check(CLI::Range(1, 1000));

    CLI11_PARSE(app, argc, argv);

    try {
        utils::display_core_info();

        if (!utils::validate_cores(small_cores)) {
            return EXIT_FAILURE;
        }

        std::cout << "Configuration:\n"
                  << "- Using cores: ";
        std::ranges::copy(small_cores, std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n- Threads: " << num_threads
                  << "\n- Problem size: " << problem_size
                  << "\n- Iterations: " << iterations
                  << "\n";

        core::thread_pool pool(small_cores);

        auto p = std::make_shared<Pipe>(
            problem_size,
            Config::DEFAULT_MIN_COORD,
            Config::DEFAULT_RANGE,
            Config::DEFAULT_SEED
        );
        
        gen_data(p, Config::DEFAULT_SEED);
        print_points(p, 5);

        // Warmup run
        cpu::dispatch_morton_code(pool, num_threads, p).wait();

        // Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < iterations; ++i) {
            cpu::dispatch_morton_code(pool, num_threads, p).wait();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        print_morton_codes(p, 5);

        double avg_ms = duration.count() / 1000.0 / iterations;
        std::cout << "\nPerformance:\n"
                  << "- Average time: " << std::fixed << std::setprecision(3) 
                  << avg_ms << " ms\n"
                  << "- Throughput: " << std::fixed << std::setprecision(2)
                  << (problem_size / avg_ms * 1000) << " points/second\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
