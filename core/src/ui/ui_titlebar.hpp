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
 * Initialize the custom titlebar system
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_titlebar_initialize(
	PFN_menu_callback callback,
	const char* app_name);

/**
 * Clean up Vulkan resources before ImGui backend shutdown
 * Called by renderer to prevent crashes during shutdown
 */
PROMETHEUS_API void ui_titlebar_cleanup_vulkan_resources();

/**
 * Shutdown the custom titlebar system
 */
PROMETHEUS_API void ui_titlebar_shutdown();

/**
 * Draw the custom titlebar
 * @param user_data - UI state pointer
 */
PROMETHEUS_API void ui_titlebar_draw();

