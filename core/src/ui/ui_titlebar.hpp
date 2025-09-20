#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;
struct ImVec2;

/**
 * Custom Titlebar Component
 * Provides a custom window titlebar with embedded icons
 * Replaces the standard OS titlebar when enabled
 */

struct UI_Titlebar_Config {
    f32 height;
    b8 show_minimize_button;
    b8 show_maximize_button;
    b8 show_close_button;
    const char* title_text;
    u32 background_color;
    u32 text_color;
    u32 button_hover_color;
    u32 button_active_color;
};

/**
 * Initialize the custom titlebar system
 * @param config - titlebar configuration
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_titlebar_initialize(const UI_Titlebar_Config* config);

/**
 * Shutdown the custom titlebar system
 */
PROMETHEUS_API void ui_titlebar_shutdown();

/**
 * Render the custom titlebar
 * @param user_data - UI state pointer
 */
PROMETHEUS_API void ui_titlebar_render(void* user_data);

/**
 * Check if titlebar area was clicked (for window dragging)
 * @param mouse_pos - current mouse position
 * @return true if titlebar was clicked
 */
PROMETHEUS_API b8 ui_titlebar_is_clicked(ImVec2 mouse_pos);

/**
 * Get titlebar height
 * @return current titlebar height
 */
PROMETHEUS_API f32 ui_titlebar_get_height();

/**
 * Set titlebar title text
 * @param title - new title text
 */
PROMETHEUS_API void ui_titlebar_set_title(const char* title);