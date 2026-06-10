#include "ProjectPath.hpp"

#include <filesystem>

namespace kk::test {

static std::string GetRootPath() {
    std::filesystem::path filePath = __FILE__;
    return std::filesystem::path(filePath.parent_path().string() + "/../../..").lexically_normal().string();
}

std::string ProjectRoot() {
    static const std::string rootPath = GetRootPath();
    return rootPath;
}

std::string AbsolutePath(const std::string &relativePath) {
    std::filesystem::path path = relativePath;
    if (path.is_absolute()) {
        return path.string();
    }
    return std::filesystem::path(ProjectRoot() + "/" + relativePath).lexically_normal().string();
}

};  // namespace kk::test
