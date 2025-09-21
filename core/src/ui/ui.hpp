#pragma once

#include "defines.hpp"
#include "ui_themes.hpp"
#include "containers/auto_array.hpp"

// Forward declarations
struct ImDrawData;
union SDL_Event;

// UI Component callback types
typedef void (*UI_RenderCallback)(void* user_data);
typedef void (*UI_AttachCallback)(void* user_data);
typedef void (*UI_DetachCallback)(void* user_data);
typedef void (*UI_MenuCallback)(void* user_data);

// UI Component definition
typedef struct UI_Component {
    const char* name;
    UI_RenderCallback on_render;
    UI_AttachCallback on_attach;    // Optional - can be nullptr
    UI_DetachCallback on_detach;    // Optional - can be nullptr
    void* user_data;                // Component-specific data
    b8 is_active;
} UI_Component;

// Internal UI state - not exposed to client
struct UI_State {
    b8 is_initialized;
    Auto_Array<UI_Component> components;
    b8 show_dockspace;
    b8 custom_titlebar_enabled;
    UI_Theme current_theme;
    UI_MenuCallback menu_callback;
    void* menu_user_data;
};

/**
 * Initialize the UI subsystem
 * @param theme - UI theme to use
 * @param enable_dockspace - true to enable docking system
 * @param enable_titlebar - true to enable custom titlebar
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_initialize(UI_Theme theme, b8 enable_dockspace, b8 enable_titlebar);

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
PROMETHEUS_API void ui_new_frame();

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
PROMETHEUS_API b8 ui_register_component(const UI_Component* component);

/**
 * Unregister a UI component by name
 * @param name - name of component to remove
 * @return true if found and removed, false otherwise
 */
PROMETHEUS_API b8 ui_unregister_component(const char* name);

/**
 * Register menu callback for the application
 * @param callback - function to call for menu rendering
 * @param user_data - user data to pass to callback
 */
PROMETHEUS_API void ui_register_menu_callback(UI_MenuCallback callback, void* user_data);

// Internal functions for core components only
/**
 * Get the current UI theme (internal use only)
 * @return current UI theme
 */
UI_Theme ui_get_current_theme();
