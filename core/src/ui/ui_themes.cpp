#include "ui_themes.hpp"
#include "imgui.h"

// Dark theme palette (current Walnut-inspired theme)
static const UI_Theme_Palette dark_palette = {
    // Primary colors
    .accent            = IM_COL32(236, 158,  36, 255),
    .highlight         = IM_COL32( 39, 185, 242, 255),
    .nice_blue         = IM_COL32( 83, 232, 254, 255),
    .compliment        = IM_COL32( 78, 151, 166, 255),

    // Background colors
    .background        = IM_COL32( 36,  36,  36, 255),
    .background_dark   = IM_COL32( 26,  26,  26, 255),
    .titlebar          = IM_COL32( 21,  21,  21, 255),
    .property_field    = IM_COL32( 15,  15,  15, 255),
    .background_popup  = IM_COL32( 50,  50,  50, 255),
    .clear_color       = IM_COL32(  0,   0,   0, 255), // Black clear color

    // Text colors
    .text              = IM_COL32(192, 192, 192, 255),
    .text_brighter     = IM_COL32(210, 210, 210, 255),
    .text_darker       = IM_COL32(128, 128, 128, 255),
    .text_error        = IM_COL32(230,  51,  51, 255),

    // UI element colors
    .muted             = IM_COL32( 77,  77,  77, 255),
    .group_header      = IM_COL32( 47,  47,  47, 255),
    .selection         = IM_COL32(237, 192, 119, 255),
    .selection_muted   = IM_COL32(237, 201, 142,  23),

    // Button colors
    .button            = IM_COL32( 56,  56,  56, 200),
    .button_hovered    = IM_COL32( 70,  70,  70, 255),
    .button_active     = IM_COL32( 56,  56,  56, 150),

    // Tab colors
    .tab_hovered       = IM_COL32(255, 225, 135,  30),
    .tab_active        = IM_COL32(255, 225, 135,  60),

    // Resize grip colors
    .resize_grip       = IM_COL32(232, 232, 232,  64), // 25% alpha
    .resize_grip_hovered = IM_COL32(207, 207, 207, 171), // 67% alpha
    .resize_grip_active  = IM_COL32(117, 117, 117, 242), // 95% alpha

    // Scrollbar colors
    .scrollbar_bg        = IM_COL32(  5,   5,   5, 135), // 53% alpha
    .scrollbar_grab      = IM_COL32( 79,  79,  79, 255),
    .scrollbar_grab_hovered = IM_COL32(105, 105, 105, 255),
    .scrollbar_grab_active  = IM_COL32(130, 130, 130, 255),

    // Separator colors
    .separator_hovered = IM_COL32( 39, 185, 242, 150)
};

// Catppuccin Mocha theme palette
static const UI_Theme_Palette catppuccin_mocha_palette = {
    // Primary colors (Catppuccin Mocha)
    .accent            = IM_COL32(245, 194, 231, 255), // Pink
    .highlight         = IM_COL32(137, 180, 250, 255), // Blue
    .nice_blue         = IM_COL32(116, 199, 236, 255), // Sky
    .compliment        = IM_COL32(148, 226, 213, 255), // Teal

    // Background colors (Catppuccin Mocha)
    .background        = IM_COL32( 49,  50,  68, 255), // Surface0
    .background_dark   = IM_COL32( 30,  30,  46, 255), // Base
    .titlebar          = IM_COL32( 24,  24,  37, 255), // Mantle
    .property_field    = IM_COL32( 17,  17,  27, 255), // Crust
    .background_popup  = IM_COL32( 69,  71,  90, 255), // Surface1
    .clear_color       = IM_COL32( 30,  30,  46, 255), // Base (same as background_dark)

    // Text colors (Catppuccin Mocha)
    .text              = IM_COL32(205, 214, 244, 255), // Text
    .text_brighter     = IM_COL32(166, 173, 200, 255), // Subtext1
    .text_darker       = IM_COL32(127, 132, 156, 255), // Subtext0
    .text_error        = IM_COL32(243, 139, 168, 255), // Red

    // UI element colors (Catppuccin Mocha)
    .muted             = IM_COL32( 88,  91, 112, 255), // Surface2
    .group_header      = IM_COL32( 69,  71,  90, 255), // Surface1
    .selection         = IM_COL32(203, 166, 247, 255), // Mauve
    .selection_muted   = IM_COL32(203, 166, 247,  60), // Mauve with alpha

    // Button colors (Catppuccin Mocha)
    .button            = IM_COL32( 69,  71,  90, 200), // Surface1 with alpha
    .button_hovered    = IM_COL32( 88,  91, 112, 255), // Surface2
    .button_active     = IM_COL32( 49,  50,  68, 255), // Surface0

    // Tab colors (Catppuccin Mocha)
    .tab_hovered       = IM_COL32(203, 166, 247,  77), // Mauve 30% alpha
    .tab_active        = IM_COL32(203, 166, 247, 153), // Mauve 60% alpha

    // Resize grip colors (Catppuccin Mocha)
    .resize_grip       = IM_COL32(166, 173, 200,  64), // Subtext1 25% alpha
    .resize_grip_hovered = IM_COL32(166, 173, 200, 171), // Subtext1 67% alpha
    .resize_grip_active  = IM_COL32(205, 214, 244, 242), // Text 95% alpha

    // Scrollbar colors (Catppuccin Mocha)
    .scrollbar_bg        = IM_COL32( 17,  17,  27, 135), // Crust 53% alpha
    .scrollbar_grab      = IM_COL32( 88,  91, 112, 255), // Surface2
    .scrollbar_grab_hovered = IM_COL32(108, 112, 134, 255), // Overlay0
    .scrollbar_grab_active  = IM_COL32(127, 132, 156, 255), // Subtext0

    // Separator colors (Catppuccin Mocha)
    .separator_hovered = IM_COL32(137, 180, 250, 150)  // Blue with alpha
};

// Theme palette array
internal_variable const UI_Theme_Palette* theme_palettes[] = {
    &dark_palette,
    &catppuccin_mocha_palette
};

// Theme names
static const char* theme_names[] = {
    "Dark",
    "Catppuccin Mocha"
};

static_assert(sizeof(theme_palettes) / sizeof(theme_palettes[0]) == (int)UI_Theme::COUNT,
              "Theme palette array size must match UI_Theme enum count");

static_assert(sizeof(theme_names) / sizeof(theme_names[0]) == (int)UI_Theme::COUNT,
              "Theme names array size must match UI_Theme enum count");

void ui_themes_apply(UI_Theme theme, ImGuiStyle& style) {
    if ((int)theme >= (int)UI_Theme::COUNT) {
        theme = UI_Theme::DARK; // Fallback to dark theme
    }

    const UI_Theme_Palette& palette = *theme_palettes[(int)theme];
    ImVec4* colors = style.Colors;


    // Apply theme colors to ImGui style - using theme-specific colors
    colors[ImGuiCol_Header]         = ImGui::ColorConvertU32ToFloat4(palette.group_header);
    colors[ImGuiCol_HeaderHovered]  = ImGui::ColorConvertU32ToFloat4(palette.group_header);
    colors[ImGuiCol_HeaderActive]   = ImGui::ColorConvertU32ToFloat4(palette.group_header);

    // Button colors - now theme-specific
    colors[ImGuiCol_Button]         = ImGui::ColorConvertU32ToFloat4(palette.button);
    colors[ImGuiCol_ButtonHovered]  = ImGui::ColorConvertU32ToFloat4(palette.button_hovered);
    colors[ImGuiCol_ButtonActive]   = ImGui::ColorConvertU32ToFloat4(palette.button_active);

    colors[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(palette.property_field);
    colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(palette.property_field);
    colors[ImGuiCol_FrameBgActive]  = ImGui::ColorConvertU32ToFloat4(palette.property_field);

    // Tab colors - now theme-specific
    colors[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(palette.titlebar);
    colors[ImGuiCol_TabHovered]         = ImGui::ColorConvertU32ToFloat4(palette.tab_hovered);
    colors[ImGuiCol_TabActive]          = ImGui::ColorConvertU32ToFloat4(palette.tab_active);
    colors[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(palette.titlebar);
    colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

    colors[ImGuiCol_TitleBg]         = ImGui::ColorConvertU32ToFloat4(palette.titlebar);
    colors[ImGuiCol_TitleBgActive]   = ImGui::ColorConvertU32ToFloat4(palette.titlebar);
    colors[ImGuiCol_TitleBgCollapsed]= ImGui::ColorConvertU32ToFloat4(palette.background_dark);

    // Resize grip colors - now theme-specific
    colors[ImGuiCol_ResizeGrip]        = ImGui::ColorConvertU32ToFloat4(palette.resize_grip);
    colors[ImGuiCol_ResizeGripHovered] = ImGui::ColorConvertU32ToFloat4(palette.resize_grip_hovered);
    colors[ImGuiCol_ResizeGripActive]  = ImGui::ColorConvertU32ToFloat4(palette.resize_grip_active);

    // Scrollbar colors - now theme-specific
    colors[ImGuiCol_ScrollbarBg]        = ImGui::ColorConvertU32ToFloat4(palette.scrollbar_bg);
    colors[ImGuiCol_ScrollbarGrab]      = ImGui::ColorConvertU32ToFloat4(palette.scrollbar_grab);
    colors[ImGuiCol_ScrollbarGrabHovered]= ImGui::ColorConvertU32ToFloat4(palette.scrollbar_grab_hovered);
    colors[ImGuiCol_ScrollbarGrabActive]= ImGui::ColorConvertU32ToFloat4(palette.scrollbar_grab_active);

    colors[ImGuiCol_Text]            = ImGui::ColorConvertU32ToFloat4(palette.text);
    colors[ImGuiCol_CheckMark]       = ImGui::ColorConvertU32ToFloat4(palette.text);
    colors[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(palette.background_dark);
    colors[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(palette.highlight);

    // Separator hover - now theme-specific
    colors[ImGuiCol_SeparatorHovered] = ImGui::ColorConvertU32ToFloat4(palette.separator_hovered);

    colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(palette.titlebar);
    colors[ImGuiCol_ChildBg]  = ImGui::ColorConvertU32ToFloat4(palette.background);
    colors[ImGuiCol_PopupBg]  = ImGui::ColorConvertU32ToFloat4(palette.background_popup);
    colors[ImGuiCol_Border]   = ImGui::ColorConvertU32ToFloat4(palette.background_dark);
    colors[ImGuiCol_TableHeaderBg]    = ImGui::ColorConvertU32ToFloat4(palette.group_header);
    colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(palette.background_dark);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Apply style tweaks
    style.FrameRounding   = 2.5f;
    style.FrameBorderSize = 1.0f;
    style.IndentSpacing   = 11.0f;
}

const char* ui_themes_get_name(UI_Theme theme) {
    if ((int)theme >= (int)UI_Theme::COUNT) {
        return "Unknown";
    }
    return theme_names[(int)theme];
}

const UI_Theme_Palette& ui_themes_get_palette(UI_Theme theme) {
    if ((int)theme >= (int)UI_Theme::COUNT) {
        theme = UI_Theme::DARK; // Fallback to dark theme
    }
    return *theme_palettes[(int)theme];
}

ImVec4 ui_themes_get_clear_color(UI_Theme theme) {
    if ((int)theme >= (int)UI_Theme::COUNT) {
        theme = UI_Theme::DARK; // Fallback to dark theme
    }
    const UI_Theme_Palette& palette = *theme_palettes[(int)theme];
    return ImGui::ColorConvertU32ToFloat4(palette.clear_color);
}
