#pragma once

#include "defines.hpp"

/**
 * Framework-Agnostic Asset System
 * Provides raw embedded asset data without framework-specific dependencies
 * UI systems handle their own framework-specific registration (ImGui, etc.)
 */

// Initialize asset system
b8 assets_initialize();

// Shutdown asset system
void assets_shutdown();

// Raw font data access (framework-agnostic)
const u8* assets_get_font_data(const char* font_name, u64* out_size);

// Forward declaration for Vulkan image
struct Vulkan_Image;

// Load image asset into Vulkan image for ImGui use
b8 assets_load_image(Vulkan_Image* out_image, const char* image_name);
