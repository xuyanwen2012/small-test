#include <gtest/gtest.h>

void pin_thread_to_core(int core_id) {
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core_id, &cpuset);

  if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
    throw std::runtime_error("Failed to pin thread to core " +
                             std::to_string(core_id));
  }
}

class ThreadPinningTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Reset CPU affinity to all cores before each test.
    //
    // Because we want to ensure that the thread is not pinned to any core
    // before we start the test.
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (int i = 0; i < 8; i++) {
      CPU_SET(i, &cpuset);
    }
    if (sched_setaffinity(0, sizeof(cpu_set_t), &cpuset) == -1) {
      throw std::runtime_error("Failed to reset CPU affinity to all cores");
    }

  }
};

TEST_F(ThreadPinningTest, PinToCore0) {
  EXPECT_NO_THROW(pin_thread_to_core(0));
}

TEST_F(ThreadPinningTest, PinToCore1) {
  EXPECT_NO_THROW(pin_thread_to_core(1));
}

TEST_F(ThreadPinningTest, PinToCore2) {
  EXPECT_NO_THROW(pin_thread_to_core(2));
}

TEST_F(ThreadPinningTest, PinToCore3) {
  EXPECT_NO_THROW(pin_thread_to_core(3));
}

TEST_F(ThreadPinningTest, PinToCore4) {
  EXPECT_NO_THROW(pin_thread_to_core(4));
}

TEST_F(ThreadPinningTest, PinToCore5) {
  EXPECT_NO_THROW(pin_thread_to_core(5));
}

TEST_F(ThreadPinningTest, PinToCore6) {
  EXPECT_NO_THROW(pin_thread_to_core(6));
}

TEST_F(ThreadPinningTest, PinToCore7) {
  EXPECT_NO_THROW(pin_thread_to_core(7));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
