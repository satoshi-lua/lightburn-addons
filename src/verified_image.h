#pragma once

#include <windows.h>
#include <bcrypt.h>
#include <filesystem>
#include <stdexcept>
#include <string>
#include "lightburn_profile.h"

#pragma comment(lib, "bcrypt.lib")

namespace lightburn {

// Hold the handle through launch and loading so the verified file cannot be
// replaced between the hash check and CreateProcess.
class ImageFile {
public:
    explicit ImageFile(const std::filesystem::path& path)
        : file(CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ,
                          nullptr, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, nullptr)) {
        if (file == INVALID_HANDLE_VALUE)
            throw std::runtime_error("Could not open the LightBurn executable or its backup for verification.");
    }
    ~ImageFile() { CloseHandle(file); }
    ImageFile(const ImageFile&) = delete;
    ImageFile& operator=(const ImageFile&) = delete;

    std::string sha256() const {
        LARGE_INTEGER start{};
        if (!SetFilePointerEx(file, start, nullptr, FILE_BEGIN))
            throw std::runtime_error("Could not seek to the start of the executable.");
        // The SHA-256 pseudo-handle is available on Windows 10 and later.
        struct Hash {
            BCRYPT_HASH_HANDLE value = nullptr;
            ~Hash() { if (value) BCryptDestroyHash(value); }
        } hash;
        if (BCryptCreateHash(BCRYPT_SHA256_ALG_HANDLE, &hash.value, nullptr, 0, nullptr, 0, 0) < 0)
            throw std::runtime_error("Could not initialize SHA-256.");
        unsigned char buffer[65536], digest[32];
        for (;;) {
            DWORD bytes = 0;
            if (!ReadFile(file, buffer, sizeof(buffer), &bytes, nullptr))
                throw std::runtime_error("Could not read the executable for verification.");
            if (!bytes) break;
            if (BCryptHashData(hash.value, buffer, bytes, 0) < 0)
                throw std::runtime_error("Could not calculate the SHA-256 hash.");
        }
        if (BCryptFinishHash(hash.value, digest, sizeof(digest), 0) < 0)
            throw std::runtime_error("Could not finalize the SHA-256 hash.");
        constexpr char hex[] = "0123456789abcdef";
        std::string result;
        result.reserve(64);
        for (auto b : digest) {
            result += hex[b >> 4];
            result += hex[b & 15];
        }
        return result;
    }

private:
    HANDLE file;
};

inline bool matchesMappedImage(HANDLE process, std::uintptr_t base, const Profile& profile) {
    auto bytes = profile.updatePrologue;
    SIZE_T read = 0;
    return base && ReadProcessMemory(process, reinterpret_cast<const void*>(base + profile.update),
                                     bytes.data(), bytes.size(), &read)
        && read == bytes.size() && bytes == profile.updatePrologue;
}

}
