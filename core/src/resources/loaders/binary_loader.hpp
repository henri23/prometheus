#pragma once

#include "defines.hpp"

/**
 * Binary Asset Loader
 * Handles loading of raw binary data (fonts, shaders, etc.)
 * Provides direct access to embedded binary assets
 */

// Initialize binary loader
b8 binary_loader_initialize();

// Shutdown binary loader
void binary_loader_shutdown();

// Get raw binary data by name
const u8* binary_loader_get_data(const char* asset_name, u64* out_size);