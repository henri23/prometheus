#include "ui_menu.hpp"
#include "ui.hpp"

#include "imgui.h"
#include "core/logger.hpp"
#include "core/asserts.hpp"
#include "memory/memory.hpp"
#include "containers/auto_array.hpp"
#include <cstdint> // For UINT32_MAX

// Menu registry (internal only)
struct UI_Menu_Registry {
    Auto_Array<UI_Menu_Item> items;
    b8 is_initialized;
};

// Internal menu system state
internal_variable UI_Menu_Registry menu_registry = {};

// Internal functions
INTERNAL_FUNC void render_menu_item(UI_Menu_Item* item);
INTERNAL_FUNC void render_menu_children(UI_Menu_Item* parent);
INTERNAL_FUNC UI_Menu_Item* find_item_recursive(UI_Menu_Item* item, const char* label);

b8 ui_menu_initialize() {
    CORE_DEBUG("Initializing menu system...");

    if (menu_registry.is_initialized) {
        CORE_WARN("Menu system already initialized");
        return true;
    }

    // Initialize menu registry (Auto_Array handles its own memory)
    menu_registry.items.clear();
    menu_registry.is_initialized = true;

    CORE_INFO("Menu system initialized successfully");
    return true;
}

void ui_menu_shutdown() {
    CORE_DEBUG("Shutting down menu system...");

    if (!menu_registry.is_initialized) {
        CORE_WARN("Menu system not initialized");
        return;
    }

    // Auto_Array destructor handles memory cleanup automatically
    menu_registry.items.clear();

    // Reset registry
    menu_registry = {};

    CORE_DEBUG("Menu system shut down successfully");
}

u32 ui_menu_create_root_item(const char* label, const char* shortcut, UI_Menu_Item_Type type) {
    RUNTIME_ASSERT_MSG(label, "Menu item label cannot be null");

    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return UINT32_MAX; // Invalid index
    }

    // Create menu item and add directly to root registry
    UI_Menu_Item item = {
        .label = label,
        .shortcut = shortcut,
        .type = type,
        .on_click = nullptr,
        .is_enabled = nullptr,
        .is_checked = nullptr,
        .user_data = nullptr,
        .enabled = true,
        .checked = false
        // Auto_Array children initializes itself automatically
    };

    menu_registry.items.push_back(item);
    u32 index = menu_registry.items.size() - 1;

    CORE_DEBUG("Created root menu item: %s at index %u", label, index);
    return index;
}

u32 ui_menu_add_child(u32 parent_index, const char* label, const char* shortcut, UI_Menu_Item_Type type) {
    RUNTIME_ASSERT_MSG(label, "Child menu item label cannot be null");

    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return UINT32_MAX;
    }

    if (parent_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid parent index: %u", parent_index);
        return UINT32_MAX;
    }

    // Create child item
    UI_Menu_Item child = {
        .label = label,
        .shortcut = shortcut,
        .type = type,
        .on_click = nullptr,
        .is_enabled = nullptr,
        .is_checked = nullptr,
        .user_data = nullptr,
        .enabled = true,
        .checked = false
    };

    // Add child to parent using Auto_Array
    UI_Menu_Item* parent = &menu_registry.items[parent_index];
    parent->children.push_back(child);
    u32 child_index = parent->children.size() - 1;

    CORE_DEBUG("Added child '%s' to parent '%s' at index %u", label, parent->label, child_index);
    return child_index;
}

void ui_menu_set_root_callback(u32 root_index, UI_MenuItem_Callback callback, void* user_data) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    UI_Menu_Item* item = &menu_registry.items[root_index];
    item->on_click = callback;
    item->user_data = user_data;
}

void ui_menu_set_child_callback(u32 root_index, u32 child_index, UI_MenuItem_Callback callback, void* user_data) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    UI_Menu_Item* parent = &menu_registry.items[root_index];
    if (child_index >= parent->children.size()) {
        CORE_ERROR("Invalid child index: %u", child_index);
        return;
    }

    UI_Menu_Item* child = &parent->children[child_index];
    child->on_click = callback;
    child->user_data = user_data;
}

void ui_menu_set_root_enabled_callback(u32 root_index, UI_MenuItem_IsEnabled_Callback callback, void* user_data) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    UI_Menu_Item* item = &menu_registry.items[root_index];
    item->is_enabled = callback;
    item->user_data = user_data;
}

void ui_menu_set_root_checked_callback(u32 root_index, UI_MenuItem_IsChecked_Callback callback, void* user_data) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    UI_Menu_Item* item = &menu_registry.items[root_index];
    item->is_checked = callback;
    item->user_data = user_data;
}

// ui_menu_register_root_item is no longer needed - use ui_menu_create_root_item instead

void ui_menu_render_all() {
    if (!menu_registry.is_initialized) {
        return;
    }

    // Render all root menu items
    for (u32 i = 0; i < menu_registry.items.size(); ++i) {
        render_menu_item(&menu_registry.items[i]);
    }
}

u32 ui_menu_find_root_item(const char* label) {
    RUNTIME_ASSERT_MSG(label, "Label cannot be null");

    if (!menu_registry.is_initialized) {
        return UINT32_MAX;
    }

    // Find by label in root items
    for (u32 i = 0; i < menu_registry.items.size(); ++i) {
        if (strcmp(menu_registry.items[i].label, label) == 0) {
            return i;
        }
    }

    return UINT32_MAX; // Not found
}

void ui_menu_set_root_enabled(u32 root_index, b8 enabled) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    menu_registry.items[root_index].enabled = enabled;
}

void ui_menu_set_root_checked(u32 root_index, b8 checked) {
    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return;
    }

    if (root_index >= menu_registry.items.size()) {
        CORE_ERROR("Invalid root index: %u", root_index);
        return;
    }

    menu_registry.items[root_index].checked = checked;
}


// Built-in menu item creators
u32 ui_menu_create_root_separator() {
    return ui_menu_create_root_item("", nullptr, UI_Menu_Item_Type::SEPARATOR);
}

u32 ui_menu_create_simple_root_item(const char* label, const char* shortcut, UI_MenuItem_Callback callback, void* user_data) {
    u32 index = ui_menu_create_root_item(label, shortcut, UI_Menu_Item_Type::ITEM);
    if (index != UINT32_MAX) {
        ui_menu_set_root_callback(index, callback, user_data);
    }
    return index;
}

u32 ui_menu_create_checkbox_root_item(const char* label, const char* shortcut, UI_MenuItem_Callback callback, UI_MenuItem_IsChecked_Callback is_checked_callback, void* user_data) {
    u32 index = ui_menu_create_root_item(label, shortcut, UI_Menu_Item_Type::CHECKBOX);
    if (index != UINT32_MAX) {
        ui_menu_set_root_callback(index, callback, user_data);
        ui_menu_set_root_checked_callback(index, is_checked_callback, user_data);
    }
    return index;
}

// Internal function implementations

INTERNAL_FUNC void render_menu_item(UI_Menu_Item* item) {
    if (!item) return;

    switch (item->type) {
        case UI_Menu_Item_Type::MENU: {
            if (ImGui::BeginMenu(item->label, item->enabled)) {
                render_menu_children(item);
                ImGui::EndMenu();
            }
            break;
        }

        case UI_Menu_Item_Type::ITEM: {
            // Determine enabled state
            b8 enabled = item->enabled;
            if (item->is_enabled) {
                enabled = item->is_enabled(item->user_data);
            }

            if (ImGui::MenuItem(item->label, item->shortcut, false, enabled)) {
                if (item->on_click) {
                    item->on_click(item->user_data);
                }
            }
            break;
        }

        case UI_Menu_Item_Type::CHECKBOX: {
            // Determine enabled and checked states
            b8 enabled = item->enabled;
            if (item->is_enabled) {
                enabled = item->is_enabled(item->user_data);
            }

            b8 checked = item->checked;
            if (item->is_checked) {
                checked = item->is_checked(item->user_data);
            }

            if (ImGui::MenuItem(item->label, item->shortcut, checked, enabled)) {
                if (item->on_click) {
                    item->on_click(item->user_data);
                }
            }
            break;
        }

        case UI_Menu_Item_Type::SEPARATOR: {
            ImGui::Separator();
            break;
        }

        case UI_Menu_Item_Type::RADIO: {
            // TODO: Implement radio button menu items
            CORE_WARN("Radio button menu items not yet implemented");
            break;
        }
    }
}

INTERNAL_FUNC void render_menu_children(UI_Menu_Item* parent) {
    if (!parent || parent->children.empty()) return;

    for (u32 i = 0; i < parent->children.size(); ++i) {
        render_menu_item(&parent->children[i]);
    }
}

// No longer needed - Auto_Array handles expansion automatically - no need for manual expansion function


INTERNAL_FUNC UI_Menu_Item* find_item_recursive(UI_Menu_Item* item, const char* label) {
    if (!item || !label) return nullptr;

    // Check if this item matches
    if (strcmp(item->label, label) == 0) {
        return item;
    }

    // Search children recursively
    for (u32 i = 0; i < item->children.size(); ++i) {
        UI_Menu_Item* found = find_item_recursive(&item->children[i], label);
        if (found) {
            return found;
        }
    }

    return nullptr;
}
