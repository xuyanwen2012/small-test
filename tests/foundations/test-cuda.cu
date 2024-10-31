#include <cuda_runtime.h>
#include <gtest/gtest.h>

#include <iostream>

// Simple CUDA kernel
__global__ void simpleKernel(int *d_data) {
  int idx = threadIdx.x;
  d_data[idx] = idx;
}

class CudaTest : public ::testing::Test {
 protected:
  int *d_data = nullptr;
  int *h_data = nullptr;
  const int dataSize = 10;

  // Set up before each test
  void SetUp() override {
    // Allocate host and device memory
    h_data = new int[dataSize];
    ASSERT_EQ(cudaMalloc(&d_data, dataSize * sizeof(int)), cudaSuccess)
        << "Failed to allocate device memory";
  }

  // Tear down after each test
  void TearDown() override {
    // Free device and host memory
    cudaFree(d_data);
    delete[] h_data;
  }
};

// Test if CUDA kernel runs correctly and produces the expected results
TEST_F(CudaTest, SimpleKernelTest) {
  // Launch kernel with 1 block of dataSize threads
  simpleKernel<<<1, dataSize>>>(d_data);

  // Copy the results back to the host
  ASSERT_EQ(cudaMemcpy(
                h_data, d_data, dataSize * sizeof(int), cudaMemcpyDeviceToHost),
            cudaSuccess)
      << "Failed to copy data from device to host";

  // Verify the result
  for (int i = 0; i < dataSize; i++) {
    EXPECT_EQ(h_data[i], i) << "Kernel output mismatch at index " << i;
  }

  // Check for any errors in kernel execution
  ASSERT_EQ(cudaGetLastError(), cudaSuccess) << "CUDA kernel execution failed";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}