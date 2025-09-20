#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;

/**
 * UI Menu System
 * Provides hierarchical menu structures with callbacks
 * Supports dynamic menu building and keyboard shortcuts
 */

// Menu item types
enum class UI_Menu_Item_Type {
    MENU,          // Submenu container
    ITEM,          // Regular menu item
    SEPARATOR,     // Menu separator
    CHECKBOX,      // Checkbox menu item
    RADIO,         // Radio button menu item (not implemented yet)
};

// Menu item callback function types
typedef void (*UI_MenuItem_Callback)(void* user_data);
typedef b8 (*UI_MenuItem_IsEnabled_Callback)(void* user_data);
typedef b8 (*UI_MenuItem_IsChecked_Callback)(void* user_data);

// Menu item structure
struct UI_Menu_Item {
    const char* label;
    const char* shortcut;
    UI_Menu_Item_Type type;
    UI_MenuItem_Callback on_click;
    UI_MenuItem_IsEnabled_Callback is_enabled;
    UI_MenuItem_IsChecked_Callback is_checked;
    void* user_data;
    b8 enabled;
    b8 checked;

    // For submenus
    struct UI_Menu_Item* children;
    u32 child_count;
    u32 child_capacity;
};


/**
 * Initialize the menu system
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_menu_initialize();

/**
 * Shutdown the menu system
 */
PROMETHEUS_API void ui_menu_shutdown();

/**
 * Create a new menu item
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param type - menu item type
 * @return pointer to created menu item or nullptr on failure
 */
PROMETHEUS_API UI_Menu_Item* ui_menu_create_item(
    const char* label,
    const char* shortcut,
    UI_Menu_Item_Type type);

/**
 * Add a child item to a menu
 * @param parent - parent menu item
 * @param child - child menu item to add
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_menu_add_child(UI_Menu_Item* parent, UI_Menu_Item* child);

/**
 * Set menu item callback
 * @param item - menu item to configure
 * @param callback - callback function
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_callback(
    UI_Menu_Item* item,
    UI_MenuItem_Callback callback,
    void* user_data);

/**
 * Set menu item enabled state callback
 * @param item - menu item to configure
 * @param callback - callback function to determine enabled state
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_enabled_callback(
    UI_Menu_Item* item,
    UI_MenuItem_IsEnabled_Callback callback,
    void* user_data);

/**
 * Set menu item checked state callback (for checkboxes)
 * @param item - menu item to configure
 * @param callback - callback function to determine checked state
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_checked_callback(
    UI_Menu_Item* item,
    UI_MenuItem_IsChecked_Callback callback,
    void* user_data);

/**
 * Register a root menu item
 * @param item - menu item to register as root
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 ui_menu_register_root_item(UI_Menu_Item* item);

/**
 * Render all registered menus
 * Called from the main menubar
 */
PROMETHEUS_API void ui_menu_render_all();

/**
 * Find a menu item by label path (e.g., "File/Open")
 * @param path - slash-separated path to menu item
 * @return pointer to menu item or nullptr if not found
 */
PROMETHEUS_API UI_Menu_Item* ui_menu_find_item(const char* path);

/**
 * Set menu item enabled state
 * @param item - menu item to modify
 * @param enabled - true to enable, false to disable
 */
PROMETHEUS_API void ui_menu_set_enabled(UI_Menu_Item* item, b8 enabled);

/**
 * Set menu item checked state
 * @param item - menu item to modify
 * @param checked - true to check, false to uncheck
 */
PROMETHEUS_API void ui_menu_set_checked(UI_Menu_Item* item, b8 checked);


// Built-in menu item creators for common functionality
/**
 * Create a separator menu item
 * @return pointer to separator item
 */
PROMETHEUS_API UI_Menu_Item* ui_menu_create_separator();

/**
 * Create a simple menu item with callback
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param callback - callback function
 * @param user_data - user data for callback
 * @return pointer to created menu item
 */
PROMETHEUS_API UI_Menu_Item* ui_menu_create_simple_item(
    const char* label,
    const char* shortcut,
    UI_MenuItem_Callback callback,
    void* user_data);

/**
 * Create a checkbox menu item
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param callback - callback function
 * @param is_checked_callback - callback to determine checked state
 * @param user_data - user data for callbacks
 * @return pointer to created menu item
 */
PROMETHEUS_API UI_Menu_Item* ui_menu_create_checkbox_item(
    const char* label,
    const char* shortcut,
    UI_MenuItem_Callback callback,
    UI_MenuItem_IsChecked_Callback is_checked_callback,
    void* user_data);