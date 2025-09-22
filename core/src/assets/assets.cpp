#include "assets.hpp"
#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "stb_image.h"
#include "defines.hpp"

// Include embedded asset data directly
#include "fonts/roboto_bold.embed"
#include "fonts/roboto_italic.embed"
#include "fonts/roboto_regular.embed"
#include "icons/prometheus_icon.embed"
#include "images/window_images.embed"
#include <cstring>

// Simple embedded asset lookup table
struct Embedded_Asset {
    const char* name;
    const u8* data;
    u64 size;
};

// All embedded assets in one table
internal_variable const Embedded_Asset embedded_assets[] = {
    {"roboto_regular",
     roboto_regular,
     sizeof(roboto_regular)},
    {"roboto_bold",
     roboto_bold,
     sizeof(roboto_bold)},
    {"roboto_italic",
     roboto_italic,
     sizeof(roboto_italic)},
    {"prometheus_icon",
     prometheus_icon,
     sizeof(prometheus_icon)},
    {"window_minimize",
     window_minimize_icon,
     sizeof(window_minimize_icon)},
    {"window_maximize",
     window_maximize_icon,
     sizeof(window_maximize_icon)},
    {"window_restore",
     window_restore_icon,
     sizeof(window_restore_icon)},
    {"window_close",
     window_close_icon,
     sizeof(window_close_icon)},
};

constexpr u32 embedded_asset_count = sizeof(embedded_assets) / sizeof(embedded_assets[0]);

// Simple asset lookup
INTERNAL_FUNC const Embedded_Asset* find_embedded_asset(const char* name) {
    for (u32 i = 0; i < embedded_asset_count; ++i) {
        if (strcmp(embedded_assets[i].name, name) == 0) {
            return &embedded_assets[i];
        }
    }
    return nullptr;
}

b8 assets_initialize() {
    CORE_DEBUG("Assets system initialized");
    return true;
}

void assets_shutdown() {
    CORE_DEBUG("Assets system shut down");
}

const u8* assets_get_font_data(const char* font_name, u64* out_size) {
    RUNTIME_ASSERT_MSG(out_size, "Output size pointer cannot be null");

    const Embedded_Asset* asset = find_embedded_asset(font_name);
    if (!asset) {
        CORE_ERROR("Font asset '%s' not found", font_name);
        *out_size = 0;
        return nullptr;
    }

    *out_size = asset->size;
    CORE_DEBUG("Retrieved font data: %s (%llu bytes)", font_name, asset->size);
    return asset->data;
}
