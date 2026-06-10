#include "Md5.hpp"

#import <CommonCrypto/CommonDigest.h>

namespace kk::test {

std::string Md5Hex(const void *bytes, size_t size) {
    if (bytes == nullptr || size == 0) {
        return {};
    }
    unsigned char digest[CC_MD5_DIGEST_LENGTH] = {};
    CC_MD5(bytes, static_cast<CC_LONG>(size), digest);
    char buffer[CC_MD5_DIGEST_LENGTH * 2 + 1] = {};
    for (size_t index = 0; index < CC_MD5_DIGEST_LENGTH; ++index) {
        snprintf(buffer + index * 2, 3, "%02x", digest[index]);
    }
    return {buffer, CC_MD5_DIGEST_LENGTH * 2};
}

};  // namespace kk::test
