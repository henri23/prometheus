#pragma once

#include "containers/auto_array.hpp"
#include "ui_themes.hpp"
#include "ui_types.hpp"

// Forward declarations
struct ImDrawData;
union SDL_Event;

/**
 * Initialize the UI subsystem
 * @param theme - UI theme to use
 * @param enable_dockspace - true to enable docking system
 * @param enable_titlebar - true to enable custom titlebar
 * @param app_name - application name for titlebar
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_initialize(
    UI_Theme theme,
	Auto_Array<UI_Layer>* layers,
	PFN_menu_callback menu_callback,
    const char* app_name);

/**
 * Shutdown the UI subsystem
 */
PROMETHEUS_API void ui_shutdown();

/**
 * Process UI events from platform layer
 * @param event - SDL event to process
 * @return true if event was consumed by UI, false to pass through
 */
PROMETHEUS_API b8 ui_process_event(const SDL_Event* event);

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
