#include "assets.hpp"
#include "core/asserts.hpp"
#include "core/logger.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "defines.hpp"
#include "renderer/vulkan/vulkan_backend.hpp"
#include "renderer/vulkan/vulkan_image.hpp"

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

b8 assets_load_image(Vulkan_Image* out_image, const char* image_name) {
    RUNTIME_ASSERT_MSG(out_image, "Output image pointer cannot be null");
    RUNTIME_ASSERT_MSG(image_name, "Image name cannot be null");

    const Embedded_Asset* asset = find_embedded_asset(image_name);
    if (!asset) {
        CORE_ERROR("Image asset '%s' not found", image_name);
        return false;
    }

    // Decode the image data using stb_image
    s32 width, height, channels;
    u8* pixel_data = stbi_load_from_memory(
        asset->data,
        (s32)asset->size,
        &width,
        &height,
        &channels,
        STBI_rgb_alpha);

    if (!pixel_data) {
        CORE_ERROR("Failed to decode image asset '%s': %s", image_name, stbi_failure_reason());
        return false;
    }

    // Calculate pixel data size
    u32 pixel_data_size = width * height * 4; // RGBA

    // Get Vulkan context
    Vulkan_Context* context = vulkan_get_context();
    if (!context) {
        CORE_ERROR("Failed to get Vulkan context for image loading");
        stbi_image_free(pixel_data);
        return false;
    }

    // Create ImGui-compatible Vulkan image
    vulkan_image_create_for_imgui(
        context,
        (u32)width,
        (u32)height,
        VK_FORMAT_R8G8B8A8_UNORM,
        pixel_data,
        pixel_data_size,
        out_image);

    // Clean up decoded pixel data
    stbi_image_free(pixel_data);

    CORE_DEBUG("Loaded image asset: %s (%dx%d)", image_name, width, height);
    return true;
}
