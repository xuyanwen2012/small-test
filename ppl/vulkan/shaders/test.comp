#version 450

layout(push_constant) uniform PushConstants { int arraySize; }
pushConstants;

layout(set = 0, binding = 0) buffer BufferA { int A[]; };

layout(set = 0, binding = 1) buffer BufferB { int B[]; };

layout(set = 0, binding = 2) buffer BufferC { int C[]; };

void main() {
  uint idx = gl_GlobalInvocationID.x;
  if (idx < pushConstants.arraySize) {
    C[idx] = A[idx] + B[idx];
  }
}
