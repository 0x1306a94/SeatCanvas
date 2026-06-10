#include "core/renderer/pass/Uniform.hpp"

#include <gtest/gtest.h>

using namespace kk::renderer;

TEST(Uniform, DefaultConstructor) {
    Uniform u;
    EXPECT_TRUE(u.empty());
    EXPECT_EQ(u.format(), UniformFormat::Float);
    EXPECT_EQ(u.count(), 1);
    EXPECT_FALSE(u.isArray());
}

TEST(Uniform, NamedConstructor) {
    Uniform u("uMVP", UniformFormat::Float4x4);
    EXPECT_FALSE(u.empty());
    EXPECT_STREQ(u.name().c_str(), "uMVP");
    EXPECT_EQ(u.format(), UniformFormat::Float4x4);
    EXPECT_EQ(u.count(), 1);
    EXPECT_FALSE(u.isArray());
}

TEST(Uniform, ArrayCount) {
    Uniform u("uValues", UniformFormat::Float3, 4);
    EXPECT_EQ(u.count(), 4);
    EXPECT_TRUE(u.isArray());
}

TEST(Uniform, IsArrayFalseForCountOne) {
    Uniform u("uValue", UniformFormat::Float, 1);
    EXPECT_FALSE(u.isArray());
}

// ---------- size() ----------

TEST(UniformSize, Float) {
    Uniform u("u", UniformFormat::Float);
    EXPECT_EQ(u.size(), sizeof(float));
}

TEST(UniformSize, Float2) {
    Uniform u("u", UniformFormat::Float2);
    EXPECT_EQ(u.size(), 2u * sizeof(float));
}

TEST(UniformSize, Float3) {
    Uniform u("u", UniformFormat::Float3);
    EXPECT_EQ(u.size(), 3u * sizeof(float));
}

TEST(UniformSize, Float4) {
    Uniform u("u", UniformFormat::Float4);
    EXPECT_EQ(u.size(), 4u * sizeof(float));
}

TEST(UniformSize, Float2x2) {
    Uniform u("u", UniformFormat::Float2x2);
    EXPECT_EQ(u.size(), 4u * sizeof(float));
}

TEST(UniformSize, Float3x3) {
    Uniform u("u", UniformFormat::Float3x3);
    EXPECT_EQ(u.size(), 9u * sizeof(float));
}

TEST(UniformSize, Float4x4) {
    Uniform u("u", UniformFormat::Float4x4);
    EXPECT_EQ(u.size(), 16u * sizeof(float));
}

TEST(UniformSize, Int) {
    Uniform u("u", UniformFormat::Int);
    EXPECT_EQ(u.size(), sizeof(int32_t));
}

TEST(UniformSize, Int2) {
    Uniform u("u", UniformFormat::Int2);
    EXPECT_EQ(u.size(), 2u * sizeof(int32_t));
}

TEST(UniformSize, Int3) {
    Uniform u("u", UniformFormat::Int3);
    EXPECT_EQ(u.size(), 3u * sizeof(int32_t));
}

TEST(UniformSize, Int4) {
    Uniform u("u", UniformFormat::Int4);
    EXPECT_EQ(u.size(), 4u * sizeof(int32_t));
}

TEST(UniformSize, Texture2DSampler) {
    Uniform u("u", UniformFormat::Texture2DSampler);
    EXPECT_EQ(u.size(), sizeof(int32_t));
}

TEST(UniformSize, TextureExternalSampler) {
    Uniform u("u", UniformFormat::TextureExternalSampler);
    EXPECT_EQ(u.size(), sizeof(int32_t));
}

TEST(UniformSize, Texture2DRectSampler) {
    Uniform u("u", UniformFormat::Texture2DRectSampler);
    EXPECT_EQ(u.size(), sizeof(int32_t));
}
