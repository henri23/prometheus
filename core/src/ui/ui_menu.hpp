#pragma once

#include "defines.hpp"
#include "containers/auto_array.hpp"

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

// Forward declaration for recursion
struct UI_Menu_Item;

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

    // For submenus - using Auto_Array for automatic memory management
    Auto_Array<UI_Menu_Item> children;
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
 * Create a new root menu item
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param type - menu item type
 * @return index of created menu item or UINT32_MAX on failure
 */
PROMETHEUS_API u32 ui_menu_create_root_item(
    const char* label,
    const char* shortcut,
    UI_Menu_Item_Type type);

/**
 * Add a child item to a root menu
 * @param parent_index - index of parent root menu item
 * @param label - child menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param type - menu item type
 * @return index of created child item or UINT32_MAX on failure
 */
PROMETHEUS_API u32 ui_menu_add_child(
    u32 parent_index,
    const char* label,
    const char* shortcut,
    UI_Menu_Item_Type type);

/**
 * Set root menu item callback
 * @param root_index - index of root menu item to configure
 * @param callback - callback function
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_root_callback(
    u32 root_index,
    UI_MenuItem_Callback callback,
    void* user_data);

/**
 * Set child menu item callback
 * @param root_index - index of root menu item
 * @param child_index - index of child menu item to configure
 * @param callback - callback function
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_child_callback(
    u32 root_index,
    u32 child_index,
    UI_MenuItem_Callback callback,
    void* user_data);

/**
 * Set root menu item enabled state callback
 * @param root_index - index of root menu item to configure
 * @param callback - callback function to determine enabled state
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_root_enabled_callback(
    u32 root_index,
    UI_MenuItem_IsEnabled_Callback callback,
    void* user_data);

/**
 * Set root menu item checked state callback (for checkboxes)
 * @param root_index - index of root menu item to configure
 * @param callback - callback function to determine checked state
 * @param user_data - user data for callback
 */
PROMETHEUS_API void ui_menu_set_root_checked_callback(
    u32 root_index,
    UI_MenuItem_IsChecked_Callback callback,
    void* user_data);

// ui_menu_register_root_item is no longer needed - use ui_menu_create_root_item instead

/**
 * Render all registered menus
 * Called from the main menubar
 */
PROMETHEUS_API void ui_menu_render_all();

/**
 * Find a root menu item by label
 * @param label - menu item label
 * @return index of menu item or UINT32_MAX if not found
 */
PROMETHEUS_API u32 ui_menu_find_root_item(const char* label);

/**
 * Set root menu item enabled state
 * @param root_index - index of root menu item to modify
 * @param enabled - true to enable, false to disable
 */
PROMETHEUS_API void ui_menu_set_root_enabled(u32 root_index, b8 enabled);

/**
 * Set root menu item checked state
 * @param root_index - index of root menu item to modify
 * @param checked - true to check, false to uncheck
 */
PROMETHEUS_API void ui_menu_set_root_checked(u32 root_index, b8 checked);


// Built-in menu item creators for common functionality
/**
 * Create a root separator menu item
 * @return index of separator item or UINT32_MAX on failure
 */
PROMETHEUS_API u32 ui_menu_create_root_separator();

/**
 * Create a simple root menu item with callback
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param callback - callback function
 * @param user_data - user data for callback
 * @return index of created menu item or UINT32_MAX on failure
 */
PROMETHEUS_API u32 ui_menu_create_simple_root_item(
    const char* label,
    const char* shortcut,
    UI_MenuItem_Callback callback,
    void* user_data);

/**
 * Create a checkbox root menu item
 * @param label - menu item label
 * @param shortcut - keyboard shortcut (optional)
 * @param callback - callback function
 * @param is_checked_callback - callback to determine checked state
 * @param user_data - user data for callbacks
 * @return index of created menu item or UINT32_MAX on failure
 */
PROMETHEUS_API u32 ui_menu_create_checkbox_root_item(
    const char* label,
    const char* shortcut,
    UI_MenuItem_Callback callback,
    UI_MenuItem_IsChecked_Callback is_checked_callback,
    void* user_data);