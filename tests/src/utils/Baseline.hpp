#pragma once

#include <memory>
#include <string>

namespace tgfx {
class Bitmap;
class Pixmap;
class Surface;
};  // namespace tgfx

namespace kk::test {

class Baseline {
  public:
    static void SetUp();
    static void TearDown();
    static void RecordFailure();

    static bool Compare(const std::shared_ptr<tgfx::Surface> &surface, const std::string &key);
    static bool Compare(const tgfx::Pixmap &pixmap, const std::string &key);
    static bool Compare(const tgfx::Bitmap &bitmap, const std::string &key);

  private:
    static bool ComparePixmap(const tgfx::Pixmap &pixmap, const std::string &key);
};

};  // namespace kk::test
