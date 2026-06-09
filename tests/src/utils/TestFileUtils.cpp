#include "TestFileUtils.hpp"

#include "ProjectPath.hpp"

#include <filesystem>
#include <fstream>

#include <tgfx/core/Bitmap.h>
#include <tgfx/core/Buffer.h>
#include <tgfx/core/Data.h>
#include <tgfx/core/ImageCodec.h>
#include <tgfx/core/Pixmap.h>
#include <tgfx/core/Stream.h>

namespace kk::test {

static constexpr char kWebpExtension[] = ".webp";

std::shared_ptr<tgfx::Data> ReadFile(const std::string &relativePath) {
    auto stream = tgfx::Stream::MakeFromFile(AbsolutePath(relativePath));
    if (stream == nullptr) {
        return nullptr;
    }
    tgfx::Buffer buffer(stream->size());
    if (!stream->read(buffer.data(), buffer.size())) {
        return nullptr;
    }
    return buffer.release();
}

void SavePixmapAsWebp(const tgfx::Pixmap &pixmap, const std::string &relativePath) {
    if (pixmap.isEmpty()) {
        return;
    }
    auto data = tgfx::ImageCodec::Encode(pixmap, tgfx::EncodedFormat::WEBP, 100);
    if (data == nullptr) {
        return;
    }
    std::filesystem::path path = AbsolutePath(relativePath);
    if (path.extension().empty()) {
        path += kWebpExtension;
    }
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char *>(data->data()), static_cast<std::streamsize>(data->size()));
}

void RemoveFile(const std::string &relativePath) {
    std::error_code errorCode = {};
    std::filesystem::remove(AbsolutePath(relativePath), errorCode);
}

};  // namespace kk::test
