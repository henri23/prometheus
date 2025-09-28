#pragma once

#include "defines.hpp"

/**
 * Image Asset Loader
 * Handles loading and decoding of image assets using STB
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

// Initialize image loader
b8 image_loader_initialize();

// Shutdown image loader
void image_loader_shutdown();

// Load and decode image asset
Image_Load_Result image_loader_load(const char* image_name);