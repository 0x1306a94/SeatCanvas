#include "core/renderer/pass/Uniform.hpp"
#include "core/renderer/pass/UniformData.hpp"

#include <cstring>
#include <gtest/gtest.h>
#include <vector>

using namespace kk::renderer;

TEST(UniformData, EmptyUniformList) {
    UniformData uniformData({});
    EXPECT_EQ(uniformData.size(), 0u);
    EXPECT_TRUE(uniformData.uniforms().empty());
}

TEST(UniformData, SingleFloat) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);
    // Float: 4 bytes, aligned to 4, total buffer padded to 16
    EXPECT_EQ(uniformData.size(), 16u);
}

TEST(UniformData, SingleFloat2) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float2)};
    UniformData uniformData(uniforms);
    // Float2: 8 bytes, aligned to 8
    EXPECT_GE(uniformData.size(), 8u);
}

TEST(UniformData, SingleFloat3) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float3)};
    UniformData uniformData(uniforms);
    // Float3: 12 bytes, aligned to 16
    EXPECT_GE(uniformData.size(), 12u);
}

TEST(UniformData, SingleFloat4) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float4)};
    UniformData uniformData(uniforms);
    // Float4: 16 bytes, aligned to 16
    EXPECT_GE(uniformData.size(), 16u);
}

TEST(UniformData, SingleInt) {
    std::vector<Uniform> uniforms = {Uniform("u_index", UniformFormat::Int)};
    UniformData uniformData(uniforms);
    // Int: 4 bytes, aligned to 4
    EXPECT_EQ(uniformData.size(), 16u);  // final size padded to 16
}

TEST(UniformData, MultipleUniformsSizeIncreases) {
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float),
        Uniform("b", UniformFormat::Float),
    };
    UniformData uniformData(uniforms);
    EXPECT_GT(uniformData.size(), 0u);
}

TEST(UniformData, FloatArray) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);
    // Array of 4 floats: each element aligned to 16 bytes, 4 * 16 = 64
    EXPECT_EQ(uniformData.size(), 64u);
}

TEST(UniformData, Float4Array) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float4, 3)};
    UniformData uniformData(uniforms);
    // Array of 3 vec4: each 16 bytes aligned to 16, 3 * 16 = 48
    EXPECT_EQ(uniformData.size(), 48u);
}

TEST(UniformData, Float3Array) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float3, 2)};
    UniformData uniformData(uniforms);
    // Array of 2 vec3: each element padded to 16 bytes, 2 * 16 = 32
    EXPECT_EQ(uniformData.size(), 32u);
}

TEST(UniformData, SetDataWritesToBuffer) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    float value = 3.14f;
    uniformData.setData("u_value", value);

    float readBack = 0.f;
    std::memcpy(&readBack, buffer, sizeof(float));
    EXPECT_FLOAT_EQ(readBack, 3.14f);
}

TEST(UniformData, SetDataInt) {
    std::vector<Uniform> uniforms = {Uniform("u_index", UniformFormat::Int)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    int32_t value = 42;
    uniformData.setData("u_index", value);

    int32_t readBack = 0;
    std::memcpy(&readBack, buffer, sizeof(int32_t));
    EXPECT_EQ(readBack, 42);
}

TEST(UniformData, SetDataFloat4) {
    std::vector<Uniform> uniforms = {Uniform("u_color", UniformFormat::Float4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);
    std::memset(buffer, 0xFF, 64);  // Fill with sentinel

    struct Vec4 {
        float x, y, z, w;
    };
    Vec4 value = {1.0f, 2.0f, 3.0f, 4.0f};
    uniformData.setData("u_color", value);

    Vec4 readBack = {};
    std::memcpy(&readBack, buffer, sizeof(Vec4));
    EXPECT_FLOAT_EQ(readBack.x, 1.0f);
    EXPECT_FLOAT_EQ(readBack.y, 2.0f);
    EXPECT_FLOAT_EQ(readBack.z, 3.0f);
    EXPECT_FLOAT_EQ(readBack.w, 4.0f);
}

TEST(UniformData, MultipleUniformsAtCorrectOffsets) {
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float),
        Uniform("b", UniformFormat::Float),
    };
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    float valueA = 1.23f;
    float valueB = 4.56f;
    uniformData.setData("a", valueA);
    uniformData.setData("b", valueB);

    // a is at offset 0, b is at offset 4
    float readA = 0.f, readB = 0.f;
    std::memcpy(&readA, buffer, sizeof(float));
    std::memcpy(&readB, buffer + 4, sizeof(float));
    EXPECT_FLOAT_EQ(readA, 1.23f);
    EXPECT_FLOAT_EQ(readB, 4.56f);
}

TEST(UniformData, SetArrayData) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);

    float values[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    uniformData.setArrayData("u_array", values, 4);

    // Each element is at aligned offset (16-byte stride)
    float readBack[4] = {};
    std::memcpy(&readBack[0], buffer, sizeof(float));
    std::memcpy(&readBack[1], buffer + 16, sizeof(float));
    std::memcpy(&readBack[2], buffer + 32, sizeof(float));
    std::memcpy(&readBack[3], buffer + 48, sizeof(float));
    EXPECT_FLOAT_EQ(readBack[0], 1.0f);
    EXPECT_FLOAT_EQ(readBack[1], 2.0f);
    EXPECT_FLOAT_EQ(readBack[2], 3.0f);
    EXPECT_FLOAT_EQ(readBack[3], 4.0f);
}

TEST(UniformData, SetArrayElement) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);

    float value = 99.9f;
    uniformData.setArrayElement("u_array", 2, &value);

    float readBack = 0.f;
    std::memcpy(&readBack, buffer + 2 * 16, sizeof(float));
    EXPECT_FLOAT_EQ(readBack, 99.9f);
}

TEST(UniformData, SetColor) {
    std::vector<Uniform> uniforms = {Uniform("u_color", UniformFormat::Float4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);
    std::memset(buffer, 0xFF, 64);

    auto color = tgfx::Color::FromRGBA(0x11, 0x22, 0x33, 0x44);
    uniformData.setData("u_color", color);

    float readBack[4] = {};
    std::memcpy(readBack, buffer, sizeof(readBack));
    EXPECT_FLOAT_EQ(readBack[0], color.red);
    EXPECT_FLOAT_EQ(readBack[1], color.green);
    EXPECT_FLOAT_EQ(readBack[2], color.blue);
    EXPECT_FLOAT_EQ(readBack[3], color.alpha);
}

TEST(UniformData, SetMatrix) {
    std::vector<Uniform> uniforms = {Uniform("u_matrix", UniformFormat::Float3x3)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);
    std::memset(buffer, 0xFF, 128);

    auto matrix = tgfx::Matrix::MakeTrans(10.f, 20.f);
    matrix.setScaleX(2.f);
    matrix.setScaleY(3.f);
    uniformData.setData("u_matrix", matrix);

    // Matrix is stored as 3 columns of 4 floats each
    float readBack[12] = {};
    std::memcpy(readBack, buffer, sizeof(readBack));

    float values[6] = {};
    matrix.get6(values);
    EXPECT_FLOAT_EQ(readBack[0], values[0]);  // scaleX
    EXPECT_FLOAT_EQ(readBack[1], values[3]);  // skewY
    EXPECT_FLOAT_EQ(readBack[4], values[1]);  // skewX
    EXPECT_FLOAT_EQ(readBack[5], values[4]);  // scaleY
    EXPECT_FLOAT_EQ(readBack[8], values[2]);  // translateX
    EXPECT_FLOAT_EQ(readBack[9], values[5]);  // translateY
    EXPECT_FLOAT_EQ(readBack[10], 1.0f);      // homogeneous
}

TEST(UniformData, SizeIsMultipleOf16) {
    // UBO requires 16-byte alignment of the total buffer size
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float),
    };
    UniformData uniformData(uniforms);
    EXPECT_EQ(uniformData.size() % 16, 0u);
}

TEST(UniformData, SingleFloat2x2) {
    std::vector<Uniform> uniforms = {Uniform("u_mat2", UniformFormat::Float2x2)};
    UniformData uniformData(uniforms);
    // Float2x2: 32 bytes, aligned to 16
    EXPECT_EQ(uniformData.size(), 32u);
}

TEST(UniformData, SingleFloat4x4) {
    std::vector<Uniform> uniforms = {Uniform("u_mat4", UniformFormat::Float4x4)};
    UniformData uniformData(uniforms);
    // Float4x4: 64 bytes, aligned to 16
    EXPECT_EQ(uniformData.size(), 64u);
}
