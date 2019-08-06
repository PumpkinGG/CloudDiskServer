#pragma once

#include <string>

namespace Utils {

// MD5 hash
std::string getStrMd5(const std::string& str);

// Transferred to Base 64
// @!!! 需要调用者free输出out
int toBase64(const char* in, size_t len, char** out);

// Transferred from Base 64
// @!!!需要调用者free输出out
int fromBase64(const char* in, size_t len, char** out);

// Aes Encrypt data transferred to Base64
std::string aesEncToBase64(const std::string& str);

// Aes Decrypt data from Base64
std::string aesDecFromBase64(const std::string& str);

} // namespace Utils