#pragma once

#include "defines.hpp"

// Forward declarations
struct ImDrawData;
union SDL_Event;

// UI subsystem interface - clean separation from renderer and platform
struct UI_State {
    b8 is_initialized;
    b8 show_demo_window;
    b8 show_simple_window;
};

/**
 * Initialize the UI subsystem
 * Should be called after renderer initialization
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_initialize();

/**
 * Shutdown the UI subsystem
 * Cleans up ImGui context and resources
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
 * Should be called at start of each frame before ui_render()
 */
PROMETHEUS_API void ui_new_frame();

/**
 * Render all UI components and prepare draw data
 * @return ImDrawData* - draw data for renderer to consume, nullptr if minimized
 */
PROMETHEUS_API ImDrawData* ui_render();

/**
 * Get current UI state for external inspection
 * @return pointer to internal UI state (read-only)
 */
PROMETHEUS_API const UI_State* ui_get_state();

// Event callback types for platform integration
typedef void (*UI_EventCallback)(const SDL_Event* event);

/**
 * Set event callback for platform integration
 * @param callback - function to call for UI event processing
 */
PROMETHEUS_API void ui_set_event_callback(UI_EventCallback callback);