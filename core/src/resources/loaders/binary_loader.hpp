#pragma once

#include "defines.hpp"

/**
 * Binary Asset Loader
 * Stateless utility for loading raw binary data (fonts, shaders, etc.)
 * Provides direct access to embedded binary assets
 */

// Get raw binary data by name
const u8* binary_loader_get_data(const char* asset_name, u64* out_size);