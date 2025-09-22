#pragma once

#include "containers/auto_array.hpp"
#include "events/events.hpp"
#include "ui_themes.hpp"
#include "ui_types.hpp"

// Forward declarations
struct ImDrawData;

/**
 * Initialize the UI subsystem
 * @param theme - UI theme to use
 * @param layers - UI layer array from client
 * @param menu_callback - callback for rendering menus
 * @param app_name - application name for titlebar
 * @param window - SDL window for ImGui initialization
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_initialize(
    UI_Theme theme,
	Auto_Array<UI_Layer>* layers,
	PFN_menu_callback menu_callback,
    const char* app_name,
    void* window);

/**
 * Shutdown the UI subsystem
 */
PROMETHEUS_API void ui_shutdown();


/**
 * Begin a new UI frame
 */
PROMETHEUS_API void ui_begin_frame();

/**
 * Render all UI components and prepare draw data
 * @return ImDrawData* - draw data for renderer to consume, nullptr if minimized
 */
PROMETHEUS_API ImDrawData* ui_render();

/**
 * Register a UI component with the system
 * @param component - component to register (copied internally)
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_register_component(const UI_Layer* component);

// Internal functions for core components only
/**
 * Get the current UI theme (internal use only)
 * @return current UI theme
 */
UI_Theme ui_get_current_theme();

/**
 * Get the UI event callback for registration by application
 * @return PFN_event_callback for UI event handling
 */
PROMETHEUS_API PFN_event_callback ui_get_event_callback();
