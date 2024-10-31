#include <gtest/gtest.h>

#include <atomic>

// #include "third-party/BS_thread_pool.hpp"

#include "core/thread_pool.hpp"

// Test case for verifying the thread pool functionality
TEST(ThreadPoolTest, BasicTaskSubmission) {
  core::thread_pool pool({0, 1, 2});

  std::atomic<int> counter(
      0);  // Atomic counter to ensure thread-safe increments

  // Submit multiple tasks to the thread pool
  std::vector<std::future<void>> futures;
  for (int i = 0; i < 10; ++i) {
    futures.push_back(pool.submit_task([&counter] { counter++; }));
  }

  // Wait for all tasks to finish by waiting on their futures
  for (auto& future : futures) {
    future.wait();
  }

  // Check if the counter was incremented 10 times
  EXPECT_EQ(counter.load(), 10);
}

// Main function to run the tests
int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
