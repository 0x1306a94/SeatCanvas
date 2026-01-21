//
//  UniformData.cpp
//  SeatCanvas
//
//  Created by king on 2025/12/18.
//

#include "UniformData.hpp"

#include <cstring>
#include <tgfx/platform/Print.h>

namespace kk::renderer {

size_t AlignTo(size_t value, size_t alignment) {
    if (alignment == 0) {
        return value;
    }
    return (value + alignment - 1) / alignment * alignment;
}

UniformData::UniformData(std::vector<Uniform> uniforms)
    : _uniforms(std::move(uniforms)) {
    // 遍历所有 uniform，计算内存布局
    for (const auto &uniform : _uniforms) {
        const auto &[entrySize, entryAlign] = EntryOf(uniform.format());

        // 计算对齐后的偏移量
        const size_t offset = alignCursor(entryAlign);

        // 计算数组大小（std140 布局规则）
        size_t totalSize = 0;
        if (uniform.isArray()) {
            // std140 布局：数组元素对齐到 vec4（16字节）边界
            // 每个元素的大小需要向上对齐到 16 字节
            // 例如：vec3 是 12 字节，但在数组中需要对齐到 16 字节
            size_t alignedElementSize = AlignTo(entrySize, 16);
            totalSize = alignedElementSize * uniform.count();
        } else {
            totalSize = entrySize;
        }

        _fieldMap[uniform.name()] = {uniform.name(), uniform.format(), uniform.count(), offset, totalSize, entryAlign};

        _cursor = offset + totalSize;
    }

    _bufferSize = alignCursor(16);
}

void UniformData::setBuffer(void *buffer) {
    _buffer = static_cast<uint8_t *>(buffer);
}

void UniformData::onSetData(const std::string &name, const void *data, size_t size) const {
    if (_buffer == nullptr) {
        tgfx::PrintError("UniformData::onSetData() buffer not set!");
        return;
    }

    const Field *field = findField(name);
    if (field == nullptr) {
        tgfx::PrintError("UniformData::onSetData() uniform '%s' not found!", name.c_str());
        return;
    }

    if (field->size != size) {
        tgfx::PrintError("UniformData::onSetData() size mismatch for '%s': expected %zu, got %zu",
                         name.c_str(), field->size, size);
        return;
    }

    std::memcpy(_buffer + field->offset, data, size);
}

void UniformData::setArrayData(const std::string &name, const void *data, int count) const {
    if (_buffer == nullptr) {
        tgfx::PrintError("UniformData::setArrayData() buffer not set!");
        return;
    }

    const Field *field = findField(name);
    if (field == nullptr) {
        tgfx::PrintError("UniformData::setArrayData() uniform '%s' not found!", name.c_str());
        return;
    }

    if (field->count <= 1) {
        tgfx::PrintError("UniformData::setArrayData() uniform '%s' is not an array!", name.c_str());
        return;
    }

    if (count > field->count) {
        tgfx::PrintError("UniformData::setArrayData() count mismatch for '%s': expected <= %d, got %d",
                         name.c_str(), field->count, count);
        return;
    }

    const auto &[entrySize, entryAlign] = EntryOf(field->format);
    size_t alignedElementSize = AlignTo(entrySize, 16);

    const uint8_t *src = static_cast<const uint8_t *>(data);
    uint8_t *dst = _buffer + field->offset;

    for (int i = 0; i < count; ++i) {
        std::memcpy(dst + i * alignedElementSize, src + i * entrySize, entrySize);
    }
}

void UniformData::setArrayElement(const std::string &name, int index, const void *data) const {
    if (_buffer == nullptr) {
        tgfx::PrintError("UniformData::setArrayElement() buffer not set!");
        return;
    }

    const Field *field = findField(name);
    if (field == nullptr) {
        tgfx::PrintError("UniformData::setArrayElement() uniform '%s' not found!", name.c_str());
        return;
    }

    if (field->count <= 1) {
        tgfx::PrintError("UniformData::setArrayElement() uniform '%s' is not an array!", name.c_str());
        return;
    }

    if (index < 0 || index >= field->count) {
        tgfx::PrintError("UniformData::setArrayElement() index out of range for '%s': index=%d, count=%d",
                         name.c_str(), index, field->count);
        return;
    }

    // 计算元素大小和对齐（std140 布局）
    const auto &[entrySize, entryAlign] = EntryOf(field->format);
    size_t alignedElementSize = AlignTo(entrySize, 16);

    uint8_t *dst = _buffer + field->offset + index * alignedElementSize;
    std::memcpy(dst, data, entrySize);
}

const UniformData::Field *UniformData::findField(const std::string &name) const {
    const auto &iter = _fieldMap.find(name);
    if (iter != _fieldMap.end()) {
        return &iter->second;
    }
    return nullptr;
}

size_t UniformData::alignCursor(size_t alignment) const {
    return AlignTo(_cursor, alignment);
}

UniformData::Entry UniformData::EntryOf(UniformFormat format) {
    switch (format) {
        case UniformFormat::Float:
            return {4, 4};  // 4 字节，4 字节对齐
        case UniformFormat::Float2:
            return {8, 8};  // 8 字节，8 字节对齐
        case UniformFormat::Float3:
            return {12, 16};  // 12 字节，16 字节对齐（vec3 需要对齐到 vec4）
        case UniformFormat::Float4:
            return {16, 16};  // 16 字节，16 字节对齐
        case UniformFormat::Float2x2:
            return {32, 16};  // 2x2 矩阵，16 字节对齐
        case UniformFormat::Float3x3:
            return {48, 16};  // 3x3 矩阵，16 字节对齐
        case UniformFormat::Float4x4:
            return {64, 16};  // 4x4 矩阵，16 字节对齐
        case UniformFormat::Int:
            return {4, 4};  // 4 字节，4 字节对齐
        case UniformFormat::Int2:
            return {8, 8};  // 8 字节，8 字节对齐
        case UniformFormat::Int3:
            return {12, 16};  // 12 字节，16 字节对齐
        case UniformFormat::Int4:
            return {16, 16};  // 16 字节，16 字节对齐
        case UniformFormat::Texture2DSampler:
        case UniformFormat::TextureExternalSampler:
        case UniformFormat::Texture2DRectSampler:
            return {4, 4};  // 采样器通常是整数索引，4 字节对齐
        default:
            return {0, 0};
    }
}

}  // namespace kk::renderer
