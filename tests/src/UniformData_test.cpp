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

// ---------- setData without buffer ----------

TEST(UniformData, SetDataWithoutBufferDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    float value = 1.0f;
    uniformData.setData("u_value", value);
    SUCCEED();
}

TEST(UniformData, SetArrayDataWithoutBufferDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    float values[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    uniformData.setArrayData("u_array", values, 4);
    SUCCEED();
}

TEST(UniformData, SetArrayElementWithoutBufferDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    float value = 42.0f;
    uniformData.setArrayElement("u_array", 2, &value);
    SUCCEED();
}

// ---------- setData with unknown name ----------

TEST(UniformData, SetDataUnknownNameDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    float value = 1.0f;
    uniformData.setData("nonexistent", value);
    SUCCEED();
}

// ---------- setData with size mismatch ----------

TEST(UniformData, SetDataSizeMismatchDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    // Float4 is 16 bytes but u_value is Float (4 bytes) — size mismatch
    struct Vec4 {
        float x, y, z, w;
    };
    Vec4 value = {1.0f, 2.0f, 3.0f, 4.0f};
    uniformData.setData("u_value", value);
    SUCCEED();
}

// ---------- setArrayData edge cases ----------

TEST(UniformData, SetArrayDataOnNonArrayDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    float values[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    uniformData.setArrayData("u_value", values, 4);
    SUCCEED();
}

TEST(UniformData, SetArrayDataCountExceedsDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);

    float values[8] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    uniformData.setArrayData("u_array", values, 8);
    SUCCEED();
}

TEST(UniformData, SetArrayDataPartial) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    std::memset(buffer, 0xFF, 128);
    uniformData.setBuffer(buffer);

    float values[2] = {10.0f, 20.0f};
    uniformData.setArrayData("u_array", values, 2);

    float readBack[2] = {};
    std::memcpy(&readBack[0], buffer, sizeof(float));
    std::memcpy(&readBack[1], buffer + 16, sizeof(float));
    EXPECT_FLOAT_EQ(readBack[0], 10.0f);
    EXPECT_FLOAT_EQ(readBack[1], 20.0f);

    // Elements 2 and 3 should be untouched (0xFF sentinels)
    uint8_t elem2 = 0;
    std::memcpy(&elem2, buffer + 32, 1);
    EXPECT_EQ(elem2, 0xFF);
}

// ---------- setArrayElement edge cases ----------

TEST(UniformData, SetArrayElementOnNonArrayDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    uniformData.setBuffer(buffer);

    float value = 42.0f;
    uniformData.setArrayElement("u_value", 0, &value);
    SUCCEED();
}

TEST(UniformData, SetArrayElementIndexNegativeDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);

    float value = 99.0f;
    uniformData.setArrayElement("u_array", -1, &value);
    SUCCEED();
}

TEST(UniformData, SetArrayElementIndexOutOfRangeDoesNotCrash) {
    std::vector<Uniform> uniforms = {Uniform("u_array", UniformFormat::Float, 4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    uniformData.setBuffer(buffer);

    float value = 99.0f;
    uniformData.setArrayElement("u_array", 4, &value);
    SUCCEED();
}

// ---------- setPMColor ----------

TEST(UniformData, SetPMColor) {
    std::vector<Uniform> uniforms = {Uniform("u_color", UniformFormat::Float4)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    tgfx::PMColor color = {0.1f, 0.2f, 0.3f, 0.4f};
    uniformData.setData("u_color", color);

    float readBack[4] = {};
    std::memcpy(readBack, buffer, sizeof(readBack));
    EXPECT_FLOAT_EQ(readBack[0], 0.1f);
    EXPECT_FLOAT_EQ(readBack[1], 0.2f);
    EXPECT_FLOAT_EQ(readBack[2], 0.3f);
    EXPECT_FLOAT_EQ(readBack[3], 0.4f);
}

// ---------- void* setData overload ----------

TEST(UniformData, SetDataViaPointer) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    float value = 7.77f;
    uniformData.setData("u_value", &value, sizeof(value));

    float readBack = 0.f;
    std::memcpy(&readBack, buffer, sizeof(float));
    EXPECT_FLOAT_EQ(readBack, 7.77f);
}

// ---------- setBuffer replaces previous ----------

TEST(UniformData, SetBufferReplaces) {
    std::vector<Uniform> uniforms = {Uniform("u_value", UniformFormat::Float)};
    UniformData uniformData(uniforms);

    uint8_t buffer1[64] = {};
    uint8_t buffer2[64] = {};
    std::memset(buffer1, 0xFF, 64);
    std::memset(buffer2, 0xFF, 64);

    uniformData.setBuffer(buffer1);
    float value1 = 1.0f;
    uniformData.setData("u_value", value1);

    // Switch to buffer2 — old data should not be affected
    uniformData.setBuffer(buffer2);
    float value2 = 2.0f;
    uniformData.setData("u_value", value2);

    // buffer1 still has 1.0
    float readB1 = 0.f;
    std::memcpy(&readB1, buffer1, sizeof(float));
    EXPECT_FLOAT_EQ(readB1, 1.0f);

    // buffer2 has 2.0
    float readB2 = 0.f;
    std::memcpy(&readB2, buffer2, sizeof(float));
    EXPECT_FLOAT_EQ(readB2, 2.0f);
}

// ---------- layout alignment for mixed types ----------

TEST(UniformData, Float3ThenFloatAlignment) {
    // Float3 (12 bytes, aligned 16) + Float (4 bytes, aligned 4)
    // Float3 at offset 0 (12 bytes, but cursor advances to 16 due to 16-byte alignment)
    // Float at offset 16
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float3),
        Uniform("b", UniformFormat::Float),
    };
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    struct Vec3 {
        float x, y, z;
    };
    Vec3 v3 = {1.0f, 2.0f, 3.0f};
    uniformData.setData("a", v3);

    float val = 4.0f;
    uniformData.setData("b", val);

    // Float3 at offset 0
    Vec3 readV3 = {};
    std::memcpy(&readV3, buffer, sizeof(Vec3));
    EXPECT_FLOAT_EQ(readV3.x, 1.0f);

    // Float3 alignment: cursor goes from 0 + 12 = 12
    // Float "b": alignCursor(4) = AlignTo(12, 4) = 12 (already aligned)
    // Float at offset 12
    float readFloat = 0.f;
    std::memcpy(&readFloat, buffer + 12, sizeof(float));
    EXPECT_FLOAT_EQ(readFloat, 4.0f);
}

TEST(UniformData, FloatThenFloat3Alignment) {
    // Float (4 bytes) + Float3 (12 bytes, aligned 16)
    // Float at offset 0, cursor = 4
    // Float3 alignment: 4 → 16 (since Float3 needs 16-byte alignment)
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float),
        Uniform("b", UniformFormat::Float3),
    };
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    float val1 = 1.0f;
    uniformData.setData("a", val1);

    struct Vec3 {
        float x, y, z;
    };
    Vec3 v3 = {2.0f, 3.0f, 4.0f};
    uniformData.setData("b", v3);

    // a at offset 0
    float readA = 0.f;
    std::memcpy(&readA, buffer, sizeof(float));
    EXPECT_FLOAT_EQ(readA, 1.0f);

    // b at offset 16 (aligned from 4 → 16)
    Vec3 readB = {};
    std::memcpy(&readB, buffer + 16, sizeof(Vec3));
    EXPECT_FLOAT_EQ(readB.x, 2.0f);
}

TEST(UniformData, Float2ThenInt2Alignment) {
    std::vector<Uniform> uniforms = {
        Uniform("a", UniformFormat::Float2),
        Uniform("b", UniformFormat::Int2),
    };
    UniformData uniformData(uniforms);

    uint8_t buffer[64] = {};
    std::memset(buffer, 0xFF, 64);
    uniformData.setBuffer(buffer);

    struct Vec2 {
        float x, y;
    };
    Vec2 v2 = {5.0f, 6.0f};
    uniformData.setData("a", v2);

    struct IVec2 {
        int32_t x, y;
    };
    IVec2 i2 = {7, 8};
    uniformData.setData("b", i2);

    Vec2 readA = {};
    std::memcpy(&readA, buffer, sizeof(Vec2));
    EXPECT_FLOAT_EQ(readA.x, 5.0f);
    EXPECT_FLOAT_EQ(readA.y, 6.0f);

    // Int2 (8 bytes, aligned 8) at offset 8 (after Float2 at 0, which is 8 bytes aligned 8)
    IVec2 readB = {};
    std::memcpy(&readB, buffer + 8, sizeof(IVec2));
    EXPECT_EQ(readB.x, 7);
    EXPECT_EQ(readB.y, 8);
}

// ---------- TextureSampler size in buffer ----------

TEST(UniformData, TextureSamplerFormat) {
    std::vector<Uniform> uniforms = {Uniform("u_tex", UniformFormat::Texture2DSampler)};
    UniformData uniformData(uniforms);
    EXPECT_EQ(uniformData.size(), 16u);
}

// ---------- Float3x3 size ----------

TEST(UniformData, SingleFloat3x3) {
    std::vector<Uniform> uniforms = {Uniform("u_mat3", UniformFormat::Float3x3)};
    UniformData uniformData(uniforms);
    // Float3x3: 48 bytes, aligned to 16, total padded to 48
    EXPECT_EQ(uniformData.size(), 48u);
}

// ---------- array of Float3 with adjacent data ----------

TEST(UniformData, ArrayFloat3ThenFloat) {
    std::vector<Uniform> uniforms = {
        Uniform("u_array", UniformFormat::Float3, 2),
        Uniform("u_value", UniformFormat::Float),
    };
    UniformData uniformData(uniforms);

    uint8_t buffer[128] = {};
    std::memset(buffer, 0xFF, 128);
    uniformData.setBuffer(buffer);

    struct Vec3 {
        float x, y, z;
    };
    // Set only first array element
    Vec3 v3 = {1.0f, 2.0f, 3.0f};
    uniformData.setArrayElement("u_array", 0, &v3);

    float val = 4.0f;
    uniformData.setData("u_value", val);

    // First array element at offset 0
    Vec3 readA0 = {};
    std::memcpy(&readA0, buffer, sizeof(Vec3));
    EXPECT_FLOAT_EQ(readA0.x, 1.0f);

    // Array of 2 Float3: each element 16 bytes → takes 32 bytes
    // "u_value" at offset 32
    float readVal = 0.f;
    std::memcpy(&readVal, buffer + 32, sizeof(float));
    EXPECT_FLOAT_EQ(readVal, 4.0f);
}

// ---------- uniform accessor ----------

TEST(UniformData, UniformsAccessorReturnsInput) {
    std::vector<Uniform> input = {
        Uniform("a", UniformFormat::Float),
        Uniform("b", UniformFormat::Float2),
    };
    UniformData uniformData(input);
    const auto &output = uniformData.uniforms();
    ASSERT_EQ(output.size(), 2u);
    EXPECT_EQ(output[0].name(), "a");
    EXPECT_EQ(output[1].name(), "b");
}

// ---------- Int3 and Int4 sizes ----------

TEST(UniformData, SingleInt3) {
    std::vector<Uniform> uniforms = {Uniform("u_val", UniformFormat::Int3)};
    UniformData uniformData(uniforms);
    // Int3: 12 bytes, aligns to 16, total padded to 16
    EXPECT_EQ(uniformData.size(), 16u);
}

TEST(UniformData, SingleInt4) {
    std::vector<Uniform> uniforms = {Uniform("u_val", UniformFormat::Int4)};
    UniformData uniformData(uniforms);
    // Int4: 16 bytes, aligns to 16
    EXPECT_EQ(uniformData.size(), 16u);
}
