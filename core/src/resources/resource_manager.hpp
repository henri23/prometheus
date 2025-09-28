#pragma once

#include "defines.hpp"

// Forward declaration for UI image resource
struct UI_Image_Resource;

/**
 * Resource Management System
 * Central registry for all resource types
 * Communicates with renderer to create GPU resources
 * Maintains clean separation between asset loading and GPU creation
 */

// Initialize resource system
b8 resource_manager_initialize();

// Shutdown resource system
void resource_manager_shutdown();

// Raw binary data access (for fonts, shaders, etc.)
const u8* resource_get_binary_data(const char* resource_name, u64* out_size);

// Image loading with renderer backend integration
b8 resource_load_image(const char* image_name, UI_Image_Resource** out_image_resource);

// Free image resource
void resource_free_image(UI_Image_Resource* resource);