#pragma once

#include "defines.hpp"

/**
 * Image Asset Loader
 * Stateless utility for loading and decoding image assets using STB
 * Provides decoded pixel data for GPU resource creation
 */

// Image load result
struct Image_Load_Result {
    u32 width;
    u32 height;
    u32 channels;
    u8* pixel_data;          // Must be freed with stbi_image_free()
    u32 pixel_data_size;
    b8 success;
    const char* error_message;
};

// Load and decode image asset
Image_Load_Result image_loader_load(const char* image_name);