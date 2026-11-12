#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include "../src/verified_image.h"
#include <cassert>
#include <iostream>

int wmain(int argc, wchar_t** argv) {
    using namespace lightburn;
    constexpr auto* baseline = findProfile("68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b");
    constexpr auto* update = findProfile("1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b");
    static_assert(baseline && std::string_view(baseline->version) == "2.1.04");
    static_assert(update && std::string_view(update->version) == "2.2.01");
    static_assert(!findProfile("1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3a"));
    static_assert(!findProfile("68a80dab"));

    // Test the mapped-image check on inert bytes in this test process.
    for (auto profile : profiles) {
        auto code = profile.updatePrologue;
        profile.update = 0;
        auto base = reinterpret_cast<std::uintptr_t>(code.data());
        assert(matchesMappedImage(GetCurrentProcess(), base, profile));
        code.back() ^= 1;
        assert(!matchesMappedImage(GetCurrentProcess(), base, profile));
        assert(!matchesMappedImage(GetCurrentProcess(), 1, profile));
    }
    assert(!findProfile(ImageFile(argv[0]).sha256()));

    if (argc == 3) {
        assert(findProfile(ImageFile(argv[1]).sha256()) == baseline);
        assert(findProfile(ImageFile(argv[2]).sha256()) == update);
    } else {
        assert(argc == 1);
    }
    std::cout << "LightBurn profile tests passed\n";
}
