#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace lightburn {

inline constexpr wchar_t bridgeName[] = L"native_bridge_v0312.dll";
inline constexpr char packageVersion[] = "0.3.12";
inline constexpr wchar_t startupErrorSuffix[] = L"-0312.error";

struct Profile {
    const char* version;
    const char* sha256;
    std::uintptr_t update;
    std::array<unsigned char, 11> updatePrologue;

    std::uintptr_t editorContext, coordinateMode, unitScale, unitSuffix;
    std::uintptr_t allocate, releaseList, flatten, limitPrimLength, buildPainterPath, computeBounds;
    std::uintptr_t undoManager, makeUndoGroup, addUndoNode, makeUndoAddShapes;
    std::uintptr_t makeUndoSelection, makeUndoDeleteShapes, submitUndo, rollbackUndo, sendEvent;
    std::uintptr_t projectPointer, getLayer, getCutSetting, isToolLayer;
    std::uintptr_t createRect, deleteOneShape, containsShape, computeRectBounds, getBounds, clearSelection, addSelection;
    std::uintptr_t sendEventWithData;
    std::uintptr_t updateToolSelection;
};

inline constexpr Profile profiles[] = {
    {
        "2.1.04", "68a80dabab4b05c3773a231d3aaaa0adf64df61b4bee2aeced6fa369b0ef149b",
        0x6834c0, {0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x10, 0x48, 0x89, 0x70, 0x18},
        0x1948bb0, 0x1948e28, 0x194935c, 0x1949368,
        0xcce530, 0x1f3b0, 0x2f36f0, 0x3043b0, 0x2e28c0, 0x2eb8d0,
        0x19493c8, 0x99530, 0x20570, 0x918a0, 0x92b50, 0x97730, 0xa06d0, 0xae370, 0x29480,
        0x1948e20, 0x328380, 0x3281c0, 0x4ce0d0,
        0x97000, 0x978e0, 0x2650c0, 0x2ebed0, 0x2f77a0, 0x23230, 0x204c0,
        0x294d0,
        0x4e9990,
    },
    {
        "2.2.01", "1c7c1d2d0843535bcd3188b569ca2b1ace5c1281eb0324de9aa881ebb65b4b3b",
        0xaf5940, {0x48, 0x8b, 0xc4, 0x55, 0x53, 0x56, 0x57, 0x41, 0x54, 0x41, 0x55},
        0x1f73f50, 0x1f741d8, 0x1f7470c, 0x1f74718,
        0x11d0890, 0xedff0, 0x5ef750, 0x600b60, 0x5ddbd0, 0x5e68d0,
        0x1f74778, 0x1c3c70, 0x118f20, 0x1bdae0, 0x1be910, 0x1c2240, 0x1cabc0, 0x1d88c0, 0x1228f0,
        0x1f741d0, 0x62db20, 0x62d110, 0x85dc30,
        0x1c1920, 0x1c23b0, 0x5357b0, 0x5e6ed0, 0x5f3470, 0x11c100, 0x118e60,
        0x122940,
        0x87d2b0,
    },
};

inline constexpr const Profile* findProfile(std::string_view sha256) {
    for (const auto& profile : profiles)
        if (sha256 == profile.sha256) return &profile;
    return nullptr;
}

}
