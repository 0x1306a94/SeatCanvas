#include "Baseline.hpp"

#include "Md5.hpp"
#include "ProjectPath.hpp"
#include "TestFileUtils.hpp"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <mutex>
#include <vector>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-warning-option"
#pragma clang diagnostic ignored "-Wdeprecated-literal-operator"
#include <nlohmann/json.hpp>
#pragma clang diagnostic pop

#include <tgfx/core/Bitmap.h>
#include <tgfx/core/Pixmap.h>
#include <tgfx/core/Surface.h>

namespace kk::test {

#ifndef SEATCANVAS_BACKEND_NAME
#define SEATCANVAS_BACKEND_NAME "metal"
#endif

static const std::string kBaselineRoot = AbsolutePath("tests/baseline/");
static const std::string kBaselineVersionPath = kBaselineRoot + "version.json";
static const std::string kCacheDir = kBaselineRoot + ".cache/" + SEATCANVAS_BACKEND_NAME + "/";
static const std::string kCacheMd5Path = kCacheDir + "md5.json";
static const std::string kCacheVersionPath = kCacheDir + "version.json";
static const std::string kGitHeadPath = kCacheDir + "HEAD";
static const std::string kOutRoot = AbsolutePath("tests/out/");
static const std::string kOutMd5Path = kOutRoot + "md5.json";
static const std::string kOutVersionPath = kOutRoot + "version.json";
static const std::string kWebpExtension = ".webp";

static nlohmann::json baselineVersion = {};
static nlohmann::json cacheVersion = {};
static nlohmann::json outputVersion = {};
static nlohmann::json cacheMd5 = {};
static nlohmann::json outputMd5 = {};
static std::mutex jsonLocker = {};
static std::string currentVersion = {};
static bool hasFailure = false;

void Baseline::RecordFailure() {
    hasFailure = true;
}

static nlohmann::json *FindJsonNode(nlohmann::json &md5Json, const std::string &key, std::string *lastKey) {
    std::vector<std::string> keys = {};
    size_t start = 0;
    size_t end = 0;
    while ((start = key.find_first_not_of('/', end)) != std::string::npos) {
        end = key.find('/', start);
        keys.push_back(key.substr(start, end - start));
    }
    *lastKey = keys.back();
    keys.pop_back();
    auto json = &md5Json;
    for (const auto &jsonKey : keys) {
        if ((*json)[jsonKey] == nullptr) {
            (*json)[jsonKey] = {};
        }
        json = &(*json)[jsonKey];
    }
    return json;
}

static std::string GetJsonValue(nlohmann::json &target, const std::string &key) {
    std::lock_guard<std::mutex> autoLock(jsonLocker);
    std::string jsonKey = {};
    auto json = FindJsonNode(target, key, &jsonKey);
    auto value = (*json)[jsonKey];
    return value != nullptr ? value.get<std::string>() : "";
}

static void SetJsonValue(nlohmann::json &target, const std::string &key, const std::string &value) {
    std::lock_guard<std::mutex> autoLock(jsonLocker);
    std::string jsonKey = {};
    auto json = FindJsonNode(target, key, &jsonKey);
    (*json)[jsonKey] = value;
}

static std::string ComputePixmapMd5(const tgfx::Pixmap &pixmap) {
    if (pixmap.rowBytes() == pixmap.info().minRowBytes()) {
        return Md5Hex(pixmap.pixels(), pixmap.byteSize());
    }
    tgfx::Bitmap newBitmap(pixmap.width(), pixmap.height(), pixmap.isAlphaOnly(), false, pixmap.colorSpace());
    tgfx::Pixmap newPixmap(newBitmap);
    if (!pixmap.readPixels(newPixmap.info(), newPixmap.writablePixels())) {
        return {};
    }
    return Md5Hex(newPixmap.pixels(), newPixmap.byteSize());
}

static bool CompareVersionAndMd5(const std::string &md5, const std::string &key,
                                 const std::function<void(bool)> &callback) {
#ifdef UPDATE_BASELINE
    SetJsonValue(outputMd5, key, md5);
    return true;
#endif
    auto expectedVersion = GetJsonValue(baselineVersion, key);
    auto cachedVersion = GetJsonValue(cacheVersion, key);
    if (expectedVersion.empty() ||
        (expectedVersion == cachedVersion && GetJsonValue(cacheMd5, key) != md5)) {
        SetJsonValue(outputVersion, key, currentVersion);
        SetJsonValue(outputMd5, key, md5);
        if (callback) {
            callback(false);
        }
        return false;
    }
    SetJsonValue(outputVersion, key, expectedVersion);
    if (callback) {
        callback(true);
    }
    return true;
}

bool Baseline::Compare(const std::shared_ptr<tgfx::Surface> &surface, const std::string &key) {
    if (surface == nullptr) {
        return false;
    }
    tgfx::Bitmap bitmap(surface->width(), surface->height(), false, false, surface->colorSpace());
    tgfx::Pixmap pixmap(bitmap);
    if (!surface->readPixels(pixmap.info(), pixmap.writablePixels())) {
        return false;
    }
    return ComparePixmap(pixmap, key);
}

bool Baseline::Compare(const tgfx::Bitmap &bitmap, const std::string &key) {
    if (bitmap.isEmpty()) {
        return false;
    }
    return ComparePixmap(tgfx::Pixmap(bitmap), key);
}

bool Baseline::Compare(const tgfx::Pixmap &pixmap, const std::string &key) {
    return ComparePixmap(pixmap, key);
}

bool Baseline::ComparePixmap(const tgfx::Pixmap &pixmap, const std::string &key) {
    if (pixmap.isEmpty()) {
        return false;
    }
    auto md5 = ComputePixmapMd5(pixmap);
    if (md5.empty()) {
        return false;
    }
    return CompareVersionAndMd5(md5, key, [key, pixmap](bool matched) {
        const auto outputPath = kOutRoot + key;
        if (matched) {
            RemoveFile(outputPath + kWebpExtension);
        } else {
            SavePixmapAsWebp(pixmap, outputPath);
        }
    });
}

void Baseline::SetUp() {
    std::ifstream cacheMd5File(kCacheMd5Path);
    if (cacheMd5File.is_open()) {
        cacheMd5File >> cacheMd5;
        cacheMd5File.close();
    }
    std::ifstream baselineVersionFile(kBaselineVersionPath);
    if (baselineVersionFile.is_open()) {
        baselineVersionFile >> baselineVersion;
        baselineVersionFile.close();
    }
    std::ifstream cacheVersionFile(kCacheVersionPath);
    if (cacheVersionFile.is_open()) {
        cacheVersionFile >> cacheVersion;
        cacheVersionFile.close();
    }
    std::ifstream headFile(kGitHeadPath);
    if (headFile.is_open()) {
        headFile >> currentVersion;
        headFile.close();
    }
}

static void RemoveEmptyFolder(const std::filesystem::path &path) {
    if (!std::filesystem::is_directory(path)) {
        if (path.filename() == ".DS_Store") {
            std::filesystem::remove(path);
        }
        return;
    }
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        RemoveEmptyFolder(entry.path());
    }
    if (std::filesystem::is_empty(path)) {
        std::filesystem::remove(path);
    }
}

static void WriteJsonFile(const std::string &path, const nlohmann::json &json) {
    std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    std::ofstream file(path);
    file << std::setw(4) << json << std::endl;
}

void Baseline::TearDown() {
#ifdef UPDATE_BASELINE
    if (!hasFailure) {
        WriteJsonFile(kCacheMd5Path, outputMd5);
        std::filesystem::copy(kBaselineVersionPath, kCacheVersionPath,
                              std::filesystem::copy_options::overwrite_existing);
    }
#else
    std::filesystem::remove(kOutMd5Path);
    if (!outputMd5.empty()) {
        WriteJsonFile(kOutMd5Path, outputMd5);
    }
    WriteJsonFile(kOutVersionPath, outputVersion);
    RemoveEmptyFolder(kOutRoot);
#endif
}

};  // namespace kk::test
