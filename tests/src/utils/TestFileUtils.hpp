#pragma once

#include <memory>
#include <string>

namespace tgfx {
class Data;
class Pixmap;
};  // namespace tgfx

namespace kk::test {

std::shared_ptr<tgfx::Data> ReadFile(const std::string &relativePath);
void SavePixmapAsWebp(const tgfx::Pixmap &pixmap, const std::string &relativePath);
void RemoveFile(const std::string &relativePath);

};  // namespace kk::test
