#pragma once

#include "defines.hpp"

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

struct UI_State {
    b8 is_initialized;
    b8 show_demo_window;
    b8 show_simple_window;

    // Component system
    UI_Component* components;
    u32 component_count;
    u32 component_capacity;

    // UI control flags
    b8 show_dockspace;
    b8 custom_titlebar_enabled;

    // Menu system
    UI_MenuCallback menu_callback;
    void* menu_user_data;
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

// Component system functions

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
 * Enable or disable a component by name
 * @param name - name of component to modify
 * @param active - true to enable, false to disable
 * @return true if found and modified, false otherwise
 */
PROMETHEUS_API b8 ui_set_component_active(const char* name, b8 active);

/**
 * Get a component by name (for inspection)
 * @param name - name of component to find
 * @return pointer to component or null if not found
 */
PROMETHEUS_API const UI_Component* ui_get_component(const char* name);

/**
 * Set the menu callback for the application
 * @param callback - function to call for menu rendering
 * @param user_data - user data to pass to callback
 */
PROMETHEUS_API void ui_set_menu_callback(UI_MenuCallback callback, void* user_data);

/**
 * Enable or disable dockspace
 * @param enabled - true to enable dockspace, false to disable
 */
PROMETHEUS_API void ui_set_dockspace_enabled(b8 enabled);

/**
 * Enable or disable custom titlebar
 * @param enabled - true to enable custom titlebar, false to disable
 */
PROMETHEUS_API void ui_set_custom_titlebar_enabled(b8 enabled);

// Forward declaration for client UI config
typedef struct Client_UI_Config Client_UI_Config;

/**
 * Set client UI configuration for component system
 * @param client_config - client UI configuration to use
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_set_client_config(Client_UI_Config* client_config);