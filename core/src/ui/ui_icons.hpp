#pragma once

#include "defines.hpp"
#include <cstring>

/**
 * Embedded UI Icons
 * Simple embedded icon data for UI components
 * Using ASCII art style for basic window controls
 */

// Icon definitions as UTF-8 strings for immediate use
namespace ui_icons {
    // Titlebar icons
    constexpr const char* MINIMIZE = "−";      // En dash (wider than hyphen)
    constexpr const char* MAXIMIZE = "□";      // Empty square
    constexpr const char* RESTORE = "❐";       // Overlapping squares
    constexpr const char* CLOSE = "✕";         // Heavy multiplication X

    // Alternative close icon (standard X)
    constexpr const char* CLOSE_ALT = "×";     // Multiplication sign

    // Menu and navigation icons
    constexpr const char* MENU = "☰";          // Trigram for heaven
    constexpr const char* ARROW_LEFT = "◀";    // Left triangle
    constexpr const char* ARROW_RIGHT = "▶";   // Right triangle
    constexpr const char* ARROW_UP = "▲";      // Up triangle
    constexpr const char* ARROW_DOWN = "▼";    // Down triangle

    // File operations
    constexpr const char* FILE_NEW = "📄";     // Page facing up
    constexpr const char* FILE_OPEN = "📁";    // File folder
    constexpr const char* FILE_SAVE = "💾";    // Floppy disk

    // Status indicators
    constexpr const char* SUCCESS = "✓";       // Check mark
    constexpr const char* WARNING = "⚠";       // Warning sign
    constexpr const char* ERROR = "⚠";         // Warning sign (reuse)
    constexpr const char* INFO = "ℹ";          // Information source

    // UI controls
    constexpr const char* SETTINGS = "⚙";      // Gear
    constexpr const char* SEARCH = "🔍";       // Magnifying glass
    constexpr const char* PLUS = "+";          // Plus sign
    constexpr const char* MINUS = "−";         // Minus sign
}

/**
 * Embedded Icon Data
 * For future use with actual image assets
 * Currently using Unicode symbols for immediate functionality
 */
struct Embedded_Icon {
    const char* name;
    const char* unicode_char;
    const u8* image_data;  // For future bitmap data
    u32 width;
    u32 height;
    u32 channels;
};

// Built-in icon registry
constexpr Embedded_Icon BUILTIN_ICONS[] = {
    { "minimize",    ui_icons::MINIMIZE,    nullptr, 0, 0, 0 },
    { "maximize",    ui_icons::MAXIMIZE,    nullptr, 0, 0, 0 },
    { "restore",     ui_icons::RESTORE,     nullptr, 0, 0, 0 },
    { "close",       ui_icons::CLOSE,       nullptr, 0, 0, 0 },
    { "close_alt",   ui_icons::CLOSE_ALT,   nullptr, 0, 0, 0 },
    { "menu",        ui_icons::MENU,        nullptr, 0, 0, 0 },
    { "arrow_left",  ui_icons::ARROW_LEFT,  nullptr, 0, 0, 0 },
    { "arrow_right", ui_icons::ARROW_RIGHT, nullptr, 0, 0, 0 },
    { "arrow_up",    ui_icons::ARROW_UP,    nullptr, 0, 0, 0 },
    { "arrow_down",  ui_icons::ARROW_DOWN,  nullptr, 0, 0, 0 },
    { "settings",    ui_icons::SETTINGS,    nullptr, 0, 0, 0 },
    { "plus",        ui_icons::PLUS,        nullptr, 0, 0, 0 },
    { "minus",       ui_icons::MINUS,       nullptr, 0, 0, 0 }
};

constexpr u32 BUILTIN_ICONS_COUNT = sizeof(BUILTIN_ICONS) / sizeof(Embedded_Icon);

/**
 * Get icon by name
 * @param name - icon name to find
 * @return pointer to icon or nullptr if not found
 */
inline const Embedded_Icon* ui_get_icon(const char* name) {
    if (!name) return nullptr;

    for (u32 i = 0; i < BUILTIN_ICONS_COUNT; ++i) {
        if (strcmp(BUILTIN_ICONS[i].name, name) == 0) {
            return &BUILTIN_ICONS[i];
        }
    }
    return nullptr;
}

/**
 * Get icon unicode character by name
 * @param name - icon name to find
 * @return unicode character string or nullptr if not found
 */
inline const char* ui_get_icon_char(const char* name) {
    const Embedded_Icon* icon = ui_get_icon(name);
    return icon ? icon->unicode_char : nullptr;
}