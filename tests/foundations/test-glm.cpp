#include <gtest/gtest.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

TEST(GLMTest, VectorAddition) {
  glm::vec3 a(1.0f, 2.0f, 3.0f);
  glm::vec3 b(4.0f, 5.0f, 6.0f);
  glm::vec3 result = a + b;
  EXPECT_FLOAT_EQ(result.x, 5.0f);
  EXPECT_FLOAT_EQ(result.y, 7.0f);
  EXPECT_FLOAT_EQ(result.z, 9.0f);
}

TEST(GLMTest, DotProduct) {
  glm::vec3 a(1.0f, 0.0f, 0.0f);
  glm::vec3 b(0.0f, 1.0f, 0.0f);
  float dotProduct = glm::dot(a, b);
  EXPECT_FLOAT_EQ(dotProduct, 0.0f);
}

TEST(GLMTest, CrossProduct) {
  glm::vec3 a(1.0f, 0.0f, 0.0f);
  glm::vec3 b(0.0f, 1.0f, 0.0f);
  glm::vec3 result = glm::cross(a, b);
  glm::vec3 expected(0.0f, 0.0f, 1.0f);
  EXPECT_FLOAT_EQ(result.x, expected.x);
  EXPECT_FLOAT_EQ(result.y, expected.y);
  EXPECT_FLOAT_EQ(result.z, expected.z);
}

TEST(GLMTest, MatrixMultiplication) {
  glm::mat4 identity = glm::mat4(1.0f);
  glm::vec4 vec(1.0f, 2.0f, 3.0f, 1.0f);
  glm::vec4 result = identity * vec;
  EXPECT_FLOAT_EQ(result.x, vec.x);
  EXPECT_FLOAT_EQ(result.y, vec.y);
  EXPECT_FLOAT_EQ(result.z, vec.z);
  EXPECT_FLOAT_EQ(result.w, vec.w);
}

TEST(GLMTest, LookAtMatrixTransformsEyeToOrigin) {
  glm::vec3 eye(0.0f, 0.0f, 1.0f);
  glm::vec3 center(0.0f, 0.0f, 0.0f);
  glm::vec3 up(0.0f, 1.0f, 0.0f);
  glm::mat4 view = glm::lookAt(eye, center, up);

  glm::vec4 transformedEye = view * glm::vec4(eye, 1.0f);

  EXPECT_NEAR(transformedEye.x, 0.0f, 1e-5f);
  EXPECT_NEAR(transformedEye.y, 0.0f, 1e-5f);
  EXPECT_NEAR(transformedEye.z, 0.0f, 1e-5f);
  EXPECT_NEAR(transformedEye.w, 1.0f, 1e-5f);
}

TEST(GLMTest, PerspectiveProjection) {
  float fov = glm::radians(45.0f);
  float aspect = 16.0f / 9.0f;
  float near = 0.1f;
  float far = 100.0f;
  glm::mat4 projection = glm::perspective(fov, aspect, near, far);

  glm::vec4 point(0.0f, 0.0f, -near, 1.0f);
  glm::vec4 projected = projection * point;
  glm::vec3 ndc = glm::vec3(projected) / projected.w;

  EXPECT_GE(ndc.x, -1.0f);
  EXPECT_LE(ndc.x, 1.0f);
  EXPECT_GE(ndc.y, -1.0f);
  EXPECT_LE(ndc.y, 1.0f);
  EXPECT_GE(ndc.z, -1.0f);
  EXPECT_LE(ndc.z, 1.0f);
}

TEST(GLMTest, NormalizeVector) {
  glm::vec3 v(1.0f, 2.0f, 3.0f);
  glm::vec3 normalized = glm::normalize(v);
  float length = glm::length(normalized);
  EXPECT_NEAR(length, 1.0f, 1e-5f);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
