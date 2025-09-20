#pragma once

#include "defines.hpp"
#include "renderer/vulkan_image.hpp"

// Forward declarations
struct ImFont;

/**
 * Simplified Asset System
 * Consolidates embedded asset loading into a single, simple interface
 * No complex registries or callback systems - just load what you need
 */

// Initialize asset system
b8 assets_initialize();

// Shutdown asset system
void assets_shutdown();

// Font loading
ImFont* assets_load_font(const char* font_name, f32 size);

// Image loading
b8 assets_load_image(Vulkan_Image* out_image, const char* asset_name);