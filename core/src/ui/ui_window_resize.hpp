#pragma once

#include "defines.hpp"

/**
 * Window Resize Handler Component
 * Handles mouse-based window resizing for borderless windows
 * Works by detecting mouse position near window edges and handling drag operations
 */

/**
 * Initialize the window resize system
 * @return true if successful, false otherwise
 */
VOLTRUM_API b8 ui_window_resize_initialize();

/**
 * Shutdown the window resize system
 */
VOLTRUM_API void ui_window_resize_shutdown();

/**
 * Handle window resize operations - should be called each frame
 * This function detects mouse position and handles resize dragging
 */
VOLTRUM_API void ui_window_resize_handle();