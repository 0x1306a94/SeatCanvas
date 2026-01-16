//
//  UniformData.hpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#ifndef UniformData_hpp
#define UniformData_hpp

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "Uniform.hpp"

#include <tgfx/core/Color.h>
#include <tgfx/core/Matrix.h>

namespace kk::renderer {

/**
 * Uniform 数据管理器
 * 参考 tgfx 的 UniformData 设计，用于管理 GPU uniform buffer 的数据布局和设置
 *
 * 核心功能：
 * - 根据 Uniform 列表自动计算内存布局和对齐
 * - 提供类型安全的 setData() 方法设置 uniform 数据
 * - 支持 Matrix 自动转换为 3x3 矩阵格式
 * - 支持设置外部缓冲区（如 GPU UBO 映射的内存）
 */
class UniformData {
  public:
    /**
     * 构造函数，根据 uniform 列表计算内存布局
     * @param uniforms uniform 变量列表
     */
    explicit UniformData(std::vector<Uniform> uniforms);

    /**
     * 设置 uniform 数据（通用模板方法）
     * @param name uniform 变量名称
     * @param value 数据值
     */
    template <typename T>
    std::enable_if_t<std::is_trivially_copyable_v<T> && !std::is_pointer_v<T> &&
                         !std::is_same_v<std::decay_t<T>, tgfx::Matrix>,
                     void>
    setData(const std::string &name, const T &value) const {
        onSetData(name, &value, sizeof(value));
    }

    void setData(const std::string &name, const void *value, size_t size) const {
        onSetData(name, value, size);
    }
    /**
     * 设置数组 uniform 数据
     * @param name uniform 变量名称
     * @param data 数组数据指针
     * @param count 数组元素数量
     */
    void setArrayData(const std::string &name, const void *data, int count) const;

    /**
     * 设置数组 uniform 中指定索引的元素
     * @param name uniform 变量名称
     * @param index 数组索引
     * @param data 元素数据指针
     */
    void setArrayElement(const std::string &name, int index, const void *data) const;

    /**
     * 设置 Matrix 数据（特殊处理，转换为 3x3 矩阵格式）
     * @param name uniform 变量名称
     * @param matrix 2D 变换矩阵
     */
    template <typename T>
    std::enable_if_t<std::is_same_v<std::decay_t<T>, tgfx::Matrix>, void> setData(
        const std::string &name, const T &matrix) const {
        float values[6] = {};
        matrix.get6(values);

        // 转换为 3x3 矩阵（按列存储，每列 4 个 float）
        const float data[] = {
            values[0], values[3], 0, 0,  // col0: scaleX, skewY, 0, 0
            values[1], values[4], 0, 0,  // col1: skewX, scaleY, 0, 0
            values[2], values[5], 1, 0,  // col2: translateX, translateY, 1, 0
        };
        onSetData(name, data, sizeof(data));
    }

    void setData(const std::string &name, const tgfx::Color &color) const {
        float values[4] = {
            color.red,
            color.green,
            color.blue,
            color.alpha,
        };

        onSetData(name, values, sizeof(values));
    }

    void setData(const std::string &name, const tgfx::PMColor &color) const {
        float values[4] = {
            color.red,
            color.green,
            color.blue,
            color.alpha,
        };

        onSetData(name, values, sizeof(values));
    }

    /**
     * 设置外部内存缓冲区
     * @param buffer 缓冲区指针（通常是 GPU UBO 映射的内存）
     */
    void setBuffer(void *buffer);

    /**
     * 获取 uniform 数据的总大小（字节数）
     */
    size_t size() const {
        return _bufferSize;
    }

    /**
     * 获取 uniform 列表
     */
    const std::vector<Uniform> &uniforms() const {
        return _uniforms;
    }

  private:
    struct Field {
        std::string name = "";
        UniformFormat format = UniformFormat::Float;
        int count = 1;      // 数组元素数量
        size_t offset = 0;  // 在缓冲区中的偏移量
        size_t size = 0;    // 数据大小（字节）
        size_t align = 0;   // 对齐要求（字节）
    };

    struct Entry {
        size_t size;   // 数据大小
        size_t align;  // 对齐要求
    };

    uint8_t *_buffer = nullptr;
    size_t _bufferSize = 0;
    std::vector<Uniform> _uniforms = {};
    std::unordered_map<std::string, Field> _fieldMap = {};
    size_t _cursor = 0;  // 当前布局游标

    void onSetData(const std::string &name, const void *data, size_t size) const;

    const Field *findField(const std::string &name) const;

    size_t alignCursor(size_t alignment) const;

    static Entry EntryOf(UniformFormat format);
};

}  // namespace kk::renderer

#endif /* UniformData_hpp */
