#include "ui_menu.hpp"
#include "ui.hpp"

#include "imgui.h"
#include "core/logger.hpp"
#include "core/asserts.hpp"
#include "memory/memory.hpp"
#include "containers/auto_array.hpp"

// Menu registry (internal only)
struct UI_Menu_Registry {
    Auto_Array<UI_Menu_Item> root_items;
    b8 is_initialized;
};

// Internal menu system state
internal_variable UI_Menu_Registry menu_registry = {};

// Internal functions
INTERNAL_FUNC UI_Menu_Item* allocate_menu_item();
INTERNAL_FUNC void deallocate_menu_item(UI_Menu_Item* item);
INTERNAL_FUNC void render_menu_item(UI_Menu_Item* item);
INTERNAL_FUNC void render_menu_children(UI_Menu_Item* parent);
INTERNAL_FUNC b8 expand_menu_item_children(UI_Menu_Item* parent);
INTERNAL_FUNC UI_Menu_Item* find_item_recursive(UI_Menu_Item* item, const char* label);

b8 ui_menu_initialize() {
    CORE_DEBUG("Initializing menu system...");

    if (menu_registry.is_initialized) {
        CORE_WARN("Menu system already initialized");
        return true;
    }

    // Initialize menu registry (Auto_Array handles its own memory)
    menu_registry.root_items.clear();
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

    // Deallocate all menu items and their children
    for (u32 i = 0; i < menu_registry.root_items.size(); ++i) {
        deallocate_menu_item(&menu_registry.root_items[i]);
    }

    // Auto_Array destructor handles memory cleanup automatically

    // Reset registry
    menu_registry = {};

    CORE_DEBUG("Menu system shut down successfully");
}

UI_Menu_Item* ui_menu_create_item(const char* label, const char* shortcut, UI_Menu_Item_Type type) {
    RUNTIME_ASSERT_MSG(label, "Menu item label cannot be null");

    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return nullptr;
    }

    UI_Menu_Item* item = allocate_menu_item();
    if (!item) {
        CORE_ERROR("Failed to allocate menu item");
        return nullptr;
    }

    // Initialize menu item
    *item = {
        .label = label,
        .shortcut = shortcut,
        .type = type,
        .on_click = nullptr,
        .is_enabled = nullptr,
        .is_checked = nullptr,
        .user_data = nullptr,
        .enabled = true,
        .checked = false,
        .children = nullptr,
        .child_count = 0,
        .child_capacity = 0
    };

    CORE_DEBUG("Created menu item: %s", label);
    return item;
}

b8 ui_menu_add_child(UI_Menu_Item* parent, UI_Menu_Item* child) {
    RUNTIME_ASSERT_MSG(parent, "Parent menu item cannot be null");
    RUNTIME_ASSERT_MSG(child, "Child menu item cannot be null");

    // Expand children array if needed
    if (parent->child_count >= parent->child_capacity) {
        if (!expand_menu_item_children(parent)) {
            return false;
        }
    }

    // Add child to parent
    parent->children[parent->child_count] = *child;
    parent->child_count++;

    CORE_DEBUG("Added child '%s' to parent '%s'", child->label, parent->label);
    return true;
}

void ui_menu_set_callback(UI_Menu_Item* item, UI_MenuItem_Callback callback, void* user_data) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");

    item->on_click = callback;
    item->user_data = user_data;
}

void ui_menu_set_enabled_callback(UI_Menu_Item* item, UI_MenuItem_IsEnabled_Callback callback, void* user_data) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");

    item->is_enabled = callback;
    item->user_data = user_data;
}

void ui_menu_set_checked_callback(UI_Menu_Item* item, UI_MenuItem_IsChecked_Callback callback, void* user_data) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");

    item->is_checked = callback;
    item->user_data = user_data;
}

b8 ui_menu_register_root_item(UI_Menu_Item* item) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");

    if (!menu_registry.is_initialized) {
        CORE_ERROR("Menu system not initialized");
        return false;
    }

    // Add item to root registry (Auto_Array handles growth automatically)
    menu_registry.root_items.push_back(*item);

    CORE_DEBUG("Registered root menu item: %s", item->label);
    return true;
}

void ui_menu_render_all() {
    if (!menu_registry.is_initialized) {
        return;
    }

    // Render all root menu items
    for (u32 i = 0; i < menu_registry.root_items.size(); ++i) {
        render_menu_item(&menu_registry.root_items[i]);
    }
}

UI_Menu_Item* ui_menu_find_item(const char* path) {
    RUNTIME_ASSERT_MSG(path, "Path cannot be null");

    if (!menu_registry.is_initialized) {
        return nullptr;
    }

    // Simple implementation: find by label in root items
    // TODO: Implement full path parsing (e.g., "File/Open")
    for (u32 i = 0; i < menu_registry.root_items.size(); ++i) {
        UI_Menu_Item* found = find_item_recursive(&menu_registry.root_items[i], path);
        if (found) {
            return found;
        }
    }

    return nullptr;
}

void ui_menu_set_enabled(UI_Menu_Item* item, b8 enabled) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");
    item->enabled = enabled;
}

void ui_menu_set_checked(UI_Menu_Item* item, b8 checked) {
    RUNTIME_ASSERT_MSG(item, "Menu item cannot be null");
    item->checked = checked;
}


// Built-in menu item creators
UI_Menu_Item* ui_menu_create_separator() {
    return ui_menu_create_item("", nullptr, UI_Menu_Item_Type::SEPARATOR);
}

UI_Menu_Item* ui_menu_create_simple_item(const char* label, const char* shortcut, UI_MenuItem_Callback callback, void* user_data) {
    UI_Menu_Item* item = ui_menu_create_item(label, shortcut, UI_Menu_Item_Type::ITEM);
    if (item) {
        ui_menu_set_callback(item, callback, user_data);
    }
    return item;
}

UI_Menu_Item* ui_menu_create_checkbox_item(const char* label, const char* shortcut, UI_MenuItem_Callback callback, UI_MenuItem_IsChecked_Callback is_checked_callback, void* user_data) {
    UI_Menu_Item* item = ui_menu_create_item(label, shortcut, UI_Menu_Item_Type::CHECKBOX);
    if (item) {
        ui_menu_set_callback(item, callback, user_data);
        ui_menu_set_checked_callback(item, is_checked_callback, user_data);
    }
    return item;
}

// Internal function implementations
INTERNAL_FUNC UI_Menu_Item* allocate_menu_item() {
    return (UI_Menu_Item*)memory_allocate(sizeof(UI_Menu_Item), Memory_Tag::UI);
}

INTERNAL_FUNC void deallocate_menu_item(UI_Menu_Item* item) {
    if (!item) return;

    // Recursively deallocate children
    for (u32 i = 0; i < item->child_count; ++i) {
        deallocate_menu_item(&item->children[i]);
    }

    // Deallocate children array
    if (item->children) {
        memory_deallocate(item->children,
                         sizeof(UI_Menu_Item) * item->child_capacity,
                         Memory_Tag::UI);
    }

    // Note: Don't deallocate the item itself if it's part of an array
}

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
    }
}

INTERNAL_FUNC void render_menu_children(UI_Menu_Item* parent) {
    if (!parent || !parent->children) return;

    for (u32 i = 0; i < parent->child_count; ++i) {
        render_menu_item(&parent->children[i]);
    }
}

INTERNAL_FUNC b8 expand_menu_item_children(UI_Menu_Item* parent) {
    u32 new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
    UI_Menu_Item* new_children = (UI_Menu_Item*)memory_allocate(
        sizeof(UI_Menu_Item) * new_capacity,
        Memory_Tag::UI);

    if (!new_children) {
        CORE_ERROR("Failed to expand menu item children");
        return false;
    }

    // Copy existing children
    for (u32 i = 0; i < parent->child_count; ++i) {
        new_children[i] = parent->children[i];
    }

    // Free old memory and update pointers
    if (parent->children) {
        memory_deallocate(parent->children,
                         sizeof(UI_Menu_Item) * parent->child_capacity,
                         Memory_Tag::UI);
    }
    parent->children = new_children;
    parent->child_capacity = new_capacity;

    return true;
}


INTERNAL_FUNC UI_Menu_Item* find_item_recursive(UI_Menu_Item* item, const char* label) {
    if (!item || !label) return nullptr;

    // Check if this item matches
    if (strcmp(item->label, label) == 0) {
        return item;
    }

    // Search children recursively
    for (u32 i = 0; i < item->child_count; ++i) {
        UI_Menu_Item* found = find_item_recursive(&item->children[i], label);
        if (found) {
            return found;
        }
    }

    return nullptr;
}