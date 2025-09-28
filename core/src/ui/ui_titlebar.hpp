#pragma once

#include "defines.hpp"
#include "ui_types.hpp"

// Forward declarations
struct UI_State;
struct ImVec2;

const f32 TITLEBAR_HEIGHT = 58.0f;
/**
 * Custom Titlebar Component
 * Provides a custom window titlebar with embedded icons and integrated menu
 * Replaces the standard OS titlebar when enabled
 */

/**
 * Setup the custom titlebar with configuration and load icons
 */
PROMETHEUS_API void ui_titlebar_setup(
	PFN_menu_callback callback,
	const char* app_name);

/**
 * Clean up renderer resources before backend shutdown
 * Called by renderer to prevent crashes during shutdown
 */
PROMETHEUS_API void ui_titlebar_cleanup_renderer_resources();

/**
 * Draw the custom titlebar
 * @param user_data - UI state pointer
 */
PROMETHEUS_API void ui_titlebar_draw();

/**
 * Check if the titlebar is currently hovered (for native window dragging)
 * @return true if titlebar is hovered, false otherwise
 */
PROMETHEUS_API b8 ui_is_titlebar_hovered();

