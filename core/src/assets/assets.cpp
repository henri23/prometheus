#include "assets.hpp"

#include "core/logger.hpp"
#include "renderer/vulkan_image.hpp"
#include "stb_image.h"

// Include embedded asset data directly
#include "fonts/roboto_bold.embed"
#include "fonts/roboto_italic.embed"
#include "fonts/roboto_regular.embed"
#include "icons/prometheus_icon.embed"
#include "images/window_images.embed"

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


b8 assets_load_image(Vulkan_Image* out_image, const char* asset_name) {
    RUNTIME_ASSERT_MSG(out_image, "Output image cannot be null");
    RUNTIME_ASSERT_MSG(asset_name, "Asset name cannot be null");

    const Embedded_Asset* asset = find_embedded_asset(asset_name);
    if (!asset) {
        CORE_ERROR("Image asset '%s' not found", asset_name);
        return false;
    }

    // Decode the image data
    u32 width, height;
    void* decoded_data = vulkan_image_decode(asset->data, asset->size, &width, &height);
    if (!decoded_data) {
        CORE_ERROR("Failed to decode image asset '%s'", asset_name);
        return false;
    }

    // Create the vulkan image
    b8 result = vulkan_image_create_from_data(
        out_image,
        width,
        height,
        Image_Format::RGBA,
        decoded_data);

    // Free the decoded data (stbi allocates this)
    stbi_image_free(decoded_data);

    if (!result) {
        CORE_ERROR("Failed to create vulkan image for asset '%s'", asset_name);
        return false;
    }

    CORE_DEBUG("Loaded image asset: %s (%ux%u)", asset_name, width, height);
    return true;
}
