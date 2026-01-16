//
//  Uniform.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef Uniform_hpp
#define Uniform_hpp

#include <string>

namespace kk::renderer {

/**
 * Uniform variable formats.
 */
enum class UniformFormat {
    Float,                   // 32-bit floating point scalar.
    Float2,                  // 2-component vector of 32-bit floating point values.
    Float3,                  // 3-component vector of 32-bit floating point values.
    Float4,                  // 4-component vector of 32-bit floating point values.
    Float2x2,                // 2x2 matrix of 32-bit floating point values.
    Float3x3,                // 3x3 matrix of 32-bit floating point values.
    Float4x4,                // 4x4 matrix of 32-bit floating point values.
    Int,                     // 32-bit signed integer scalar.
    Int2,                    // 2-component vector of 32-bit signed integer values.
    Int3,                    // 3-component vector of 32-bit signed integer values.
    Int4,                    // 4-component vector of 32-bit signed integer values.
    Texture2DSampler,        // 2D texture sampler.
    TextureExternalSampler,  // External texture sampler (e.g. for camera input).
    Texture2DRectSampler,    // Rectangle texture sampler.
};

/**
 * Represents a uniform variable in a GPU program.
 */
class Uniform {
  public:
    /**
     * Default constructor for Uniform.
     */
    Uniform() = default;

    /**
     * Creates a uniform variable with the specified name and format.
     * @param name uniform 变量名称
     * @param format uniform 变量格式
     * @param count 数组元素数量，默认为 1（非数组）
     */
    Uniform(std::string name, UniformFormat format, int count = 1)
        : _name(std::move(name))
        , _format(format)
        , _count(count) {
    }

    /**
     * Returns true if the uniform variable is empty.
     */
    bool empty() const {
        return _name.empty();
    }

    /**
     * The name of the uniform variable.
     */
    const std::string &name() const {
        return _name;
    }

    /**
     * The format of the uniform variable.
     */
    UniformFormat format() const {
        return _format;
    }

    /**
     * The array count of the uniform variable.
     * Returns 1 for non-array uniforms.
     */
    int count() const {
        return _count;
    }

    /**
     * Returns true if this is an array uniform.
     */
    bool isArray() const {
        return _count > 1;
    }

    /**
     * Returns the size of the uniform variable in bytes.
     */
    size_t size() const;

  private:
    std::string _name = {};
    UniformFormat _format = UniformFormat::Float;
    int _count = 1;  // 数组元素数量，默认为 1（非数组）
};

}  // namespace kk::renderer

#endif /* Uniform_hpp */
