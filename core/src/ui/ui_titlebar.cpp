#include "ui_titlebar.hpp"
#include "assets/assets.hpp"
#include "ui.hpp"
#include "ui_dockspace.hpp"
#include "ui_themes.hpp"

#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "imgui.h"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/vulkan_image.hpp"

// Internal titlebar state
struct Titlebar_State {
    b8 is_initialized;
    const char* title_text;
    f32 current_height;
    ImVec2 titlebar_min;
    ImVec2 titlebar_max;
    b8 is_dragging;
    ImVec2 drag_offset;
    b8 show_menus;

    // Icons loaded directly
    Vulkan_Image app_icon;
    Vulkan_Image minimize_icon;
    Vulkan_Image maximize_icon;
    Vulkan_Image restore_icon;
    Vulkan_Image close_icon;
    b8 icons_loaded;
};

internal_variable Titlebar_State titlebar_state = {};

// Titlebar constants
static const f32 TITLEBAR_HEIGHT = 50.0f;
static const char* DEFAULT_TITLE = "Prometheus Engine";

// Internal functions
INTERNAL_FUNC void render_titlebar_background();
INTERNAL_FUNC void render_titlebar_gradient();
INTERNAL_FUNC void render_titlebar_logo();
INTERNAL_FUNC void render_titlebar_menus(UI_State* ui_state);
INTERNAL_FUNC void render_titlebar_text();
INTERNAL_FUNC void render_titlebar_buttons();

INTERNAL_FUNC b8 render_titlebar_button(const char* icon,
    ImVec2 pos,
    ImVec2 size);

INTERNAL_FUNC b8 render_titlebar_image_button(const Vulkan_Image* image,
    const char* fallback_text,
    ImVec2 pos,
    ImVec2 size);

b8 ui_titlebar_initialize() {
    CORE_DEBUG("Initializing custom titlebar...");

    if (titlebar_state.is_initialized) {
        CORE_WARN("Titlebar already initialized");
        return true;
    }

    // Initialize with defaults
    titlebar_state.title_text = DEFAULT_TITLE;
    titlebar_state.current_height = TITLEBAR_HEIGHT;
    titlebar_state.is_dragging = false;
    titlebar_state.drag_offset = ImVec2(0, 0);
    titlebar_state.show_menus = true;
    titlebar_state.is_initialized = true;

    // Disable dockspace menubar since we're using titlebar menus by default
    extern void ui_dockspace_set_show_menubar(b8 show_menubar);
    ui_dockspace_set_show_menubar(false);

    // Load icons directly
    b8 icons_success = true;
    icons_success &=
        assets_load_image(&titlebar_state.app_icon, "prometheus_icon");
    icons_success &=
        assets_load_image(&titlebar_state.minimize_icon, "window_minimize");
    icons_success &=
        assets_load_image(&titlebar_state.maximize_icon, "window_maximize");
    icons_success &=
        assets_load_image(&titlebar_state.restore_icon, "window_restore");
    icons_success &=
        assets_load_image(&titlebar_state.close_icon, "window_close");

    titlebar_state.icons_loaded = icons_success;

    if (icons_success) {
        CORE_INFO("All titlebar icons loaded successfully");
    } else {
        CORE_WARN("Some titlebar icons failed to load, using fallback text");
    }

    CORE_INFO("Custom titlebar initialized with height %.1f",
        titlebar_state.current_height);
    return true;
}

void ui_titlebar_shutdown() {
    CORE_DEBUG("Shutting down custom titlebar...");

    if (!titlebar_state.is_initialized) {
        CORE_WARN("Titlebar not initialized");
        return;
    }

    // Cleanup icons if loaded
    if (titlebar_state.icons_loaded) {
        vulkan_image_destroy(&titlebar_state.app_icon);
        vulkan_image_destroy(&titlebar_state.minimize_icon);
        vulkan_image_destroy(&titlebar_state.maximize_icon);
        vulkan_image_destroy(&titlebar_state.restore_icon);
        vulkan_image_destroy(&titlebar_state.close_icon);
    }

    // Reset state
    titlebar_state = {};

    CORE_DEBUG("Custom titlebar shut down successfully");
}

void ui_titlebar_render(void* user_data) {
    if (!titlebar_state.is_initialized) {
        CORE_DEBUG("Titlebar not initialized, skipping render");
        return;
    }

    UI_State* ui_state = (UI_State*)user_data;
    if (!ui_state || !ui_state->is_initialized ||
        !ui_state->custom_titlebar_enabled) {
        CORE_DEBUG(
            "UI state check failed: ui_state=%p, ui_initialized=%s, "
            "titlebar_enabled=%s",
            (void*)ui_state,
            ui_state ? (ui_state->is_initialized ? "true" : "false") : "null",
            ui_state ? (ui_state->custom_titlebar_enabled ? "true" : "false")
                     : "null");
        return;
    }

    // Colors are now fetched directly from theme in render functions

    static b8 render_debug_logged = false;
    if (!render_debug_logged) {
        CORE_DEBUG("Custom titlebar rendering started");
        render_debug_logged = true;
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 window_pos = viewport->Pos;
    ImVec2 window_size = viewport->Size;

    // Set titlebar bounds
    titlebar_state.titlebar_min = window_pos;
    titlebar_state.titlebar_max = ImVec2(window_pos.x + window_size.x,
        window_pos.y + titlebar_state.current_height);

    // Create invisible window for titlebar
    ImGui::SetNextWindowPos(titlebar_state.titlebar_min);
    ImGui::SetNextWindowSize(
        ImVec2(window_size.x, titlebar_state.current_height));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##CustomTitlebar", nullptr, flags)) {
        render_titlebar_background();
        render_titlebar_gradient();
        render_titlebar_logo();
        if (titlebar_state.show_menus) {
            render_titlebar_menus(ui_state);
        }
        render_titlebar_text();
        render_titlebar_buttons();
    }
    ImGui::End();
}

b8 ui_titlebar_is_clicked(ImVec2 mouse_pos) {
    if (!titlebar_state.is_initialized) {
        return false;
    }

    return (mouse_pos.x >= titlebar_state.titlebar_min.x &&
            mouse_pos.x <= titlebar_state.titlebar_max.x &&
            mouse_pos.y >= titlebar_state.titlebar_min.y &&
            mouse_pos.y <= titlebar_state.titlebar_max.y);
}

f32 ui_titlebar_get_height() {
    return titlebar_state.is_initialized ? titlebar_state.current_height : 0.0f;
}

void ui_titlebar_set_title(const char* title) {
    RUNTIME_ASSERT_MSG(title, "Title cannot be null");

    if (titlebar_state.is_initialized) {
        titlebar_state.title_text = title;
        CORE_DEBUG("Titlebar title set to: %s", title);
    }
}

void ui_titlebar_set_show_menus(b8 show_menus) {
    if (titlebar_state.is_initialized) {
        titlebar_state.show_menus = show_menus;
        CORE_DEBUG("Titlebar menus %s", show_menus ? "enabled" : "disabled");

        // Automatically disable dockspace menubar when titlebar menus are enabled
        // to avoid having both menubar types active simultaneously
        extern void ui_dockspace_set_show_menubar(b8 show_menubar);
        ui_dockspace_set_show_menubar(!show_menus);
    }
}

// Internal function implementations
INTERNAL_FUNC const UI_Theme_Palette& get_current_palette() {
    extern UI_Theme ui_get_current_theme(); // Internal function from ui.cpp
    UI_Theme current_theme = ui_get_current_theme();
    return ui_themes_get_palette(current_theme);
}

INTERNAL_FUNC void render_titlebar_background() {
    const UI_Theme_Palette& palette = get_current_palette();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(titlebar_state.titlebar_min,
        titlebar_state.titlebar_max,
        palette.titlebar);
}

INTERNAL_FUNC void render_titlebar_logo() {
    const Vulkan_Image* app_icon =
        titlebar_state.icons_loaded ? &titlebar_state.app_icon : nullptr;
    b8 icons_loaded = titlebar_state.icons_loaded;

    // Debug output to see what's happening
    static b8 debug_logged = false;
    if (!debug_logged) {
        CORE_DEBUG(
            "Titlebar logo debug: app_icon=%p, icons_loaded=%s, "
            "descriptor_set=%p",
            (void*)app_icon,
            icons_loaded ? "true" : "false",
            app_icon ? (void*)app_icon->descriptor_set : nullptr);
        debug_logged = true;
    }

    // Fixed logo size (decoupled from titlebar height)
    f32 logo_size = 32.0f; // Fixed size for larger Prometheus icon
    f32 logo_margin = 8.0f;
    f32 logo_top_padding = 2.0f; // Same top padding as menu entries and window buttons

    // Position: fixed distance from top edge (not centered)
    ImVec2 logo_pos = ImVec2(titlebar_state.titlebar_min.x + logo_margin,
        titlebar_state.titlebar_min.y + logo_top_padding);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    if (app_icon && icons_loaded &&
        app_icon->descriptor_set != VK_NULL_HANDLE) {
        // Render the scalable logo image
        draw_list->AddImage((ImTextureID)(intptr_t)app_icon->descriptor_set,
            logo_pos,
            ImVec2(logo_pos.x + logo_size, logo_pos.y + logo_size),
            ImVec2(0, 0),
            ImVec2(1, 1),
            IM_COL32_WHITE);
    } else {
        // Fallback: draw a scalable placeholder
        const UI_Theme_Palette& palette = get_current_palette();
        draw_list->AddRect(logo_pos,
            ImVec2(logo_pos.x + logo_size, logo_pos.y + logo_size),
            palette.text,
            2.0f);

        // Add "P" text as placeholder
        ImVec2 text_size = ImGui::CalcTextSize("P");
        ImVec2 text_pos = ImVec2(logo_pos.x + (logo_size - text_size.x) * 0.5f,
            logo_pos.y + (logo_size - text_size.y) * 0.5f);
        draw_list->AddText(text_pos, palette.text, "P");
    }
}

INTERNAL_FUNC void render_titlebar_menus(UI_State* ui_state) {
    if (!ui_state || !ui_state->menu_callback) {
        return; // No menu callback registered
    }

    const UI_Theme_Palette& palette = get_current_palette();

    // Calculate menu position (after logo)
    f32 logo_size = 32.0f;
    f32 logo_margin = 8.0f;
    f32 menu_start_x = titlebar_state.titlebar_min.x + logo_margin + logo_size + 12.0f;

    // Top-align menus like window buttons - same padding as window buttons
    f32 menu_top_padding = 2.0f; // Same as window buttons top_padding
    f32 menu_y_top_aligned = titlebar_state.titlebar_min.y + menu_top_padding;

    // Push menu styling - no outline, just color changes, same height as window buttons
    f32 button_size = 26.0f; // Same as window buttons
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0)); // Transparent
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 30)); // Subtle white hover
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, palette.highlight); // Theme highlight when clicked
    ImGui::PushStyleColor(ImGuiCol_Text, palette.text); // Theme text color
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 2)); // Minimal vertical padding to match window buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f); // No rounded corners
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // No border/outline

    f32 current_x = menu_start_x;

    // File menu
    ImGui::SetCursorScreenPos(ImVec2(current_x, menu_y_top_aligned));
    if (ImGui::Button("File")) {
        ImGui::OpenPopup("FileMenu");
    }
    if (ImGui::BeginPopup("FileMenu")) {
        // Style the popup to match theme with proper padding
        ImGui::PushStyleColor(ImGuiCol_PopupBg, palette.background_popup);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, palette.button_hovered);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 8)); // Popup window padding
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

        if (ImGui::MenuItem("New", "Ctrl+N")) {
            // TODO: Handle File->New
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
            // TODO: Handle File->Open
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            // TODO: Handle File->Save
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) {
            // TODO: Handle File->Exit
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
    current_x += ImGui::CalcTextSize("File").x + 16; // text width + padding, no extra spacing

    // View menu - get client state through callback data
    ImGui::SetCursorScreenPos(ImVec2(current_x, menu_y_top_aligned));
    if (ImGui::Button("View")) {
        ImGui::OpenPopup("ViewMenu");
    }
    if (ImGui::BeginPopup("ViewMenu")) {
        // Style the popup to match theme with proper padding
        ImGui::PushStyleColor(ImGuiCol_PopupBg, palette.background_popup);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, palette.button_hovered);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 8)); // Popup window padding
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

        // Access client state through the menu callback user data
        // We need to cast it to the client state structure (matching client_ui.cpp)
        typedef struct {
            b8 is_initialized;
            f32 slider_value;
            s32 counter;
            void* clear_color;
            b8 show_prometheus_window;
            b8 show_demo_window;
        } Client_UI_State;

        if (ui_state->menu_user_data) {
            Client_UI_State* client_state = (Client_UI_State*)ui_state->menu_user_data;

            ImGui::MenuItem("Prometheus Window", nullptr, &client_state->show_prometheus_window);
            ImGui::MenuItem("Demo Window", nullptr, &client_state->show_demo_window);
        } else {
            // Fallback static variables
            static bool show_prometheus_window = true;
            static bool show_demo_window = false;
            ImGui::MenuItem("Prometheus Window", nullptr, &show_prometheus_window);
            ImGui::MenuItem("Demo Window", nullptr, &show_demo_window);
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Reset Layout")) {
            ui_dockspace_reset_layout();
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }
    current_x += ImGui::CalcTextSize("View").x + 16; // text width + padding, no extra spacing

    // Help menu
    ImGui::SetCursorScreenPos(ImVec2(current_x, menu_y_top_aligned));
    if (ImGui::Button("Help")) {
        ImGui::OpenPopup("HelpMenu");
    }
    if (ImGui::BeginPopup("HelpMenu")) {
        // Style the popup to match theme with proper padding
        ImGui::PushStyleColor(ImGuiCol_PopupBg, palette.background_popup);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, palette.button_hovered);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12, 8)); // Popup window padding
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 4));

        if (ImGui::MenuItem("About")) {
            // TODO: Handle Help->About
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        ImGui::EndPopup();
    }

    // Ensure the demo window is shown if enabled by calling client logic
    if (ui_state->menu_user_data) {
        typedef struct {
            b8 is_initialized;
            f32 slider_value;
            s32 counter;
            void* clear_color;
            b8 show_prometheus_window;
            b8 show_demo_window;
        } Client_UI_State;

        Client_UI_State* client_state = (Client_UI_State*)ui_state->menu_user_data;

        // Show demo window if enabled
        if (client_state->show_demo_window) {
            ImGui::ShowDemoWindow(&client_state->show_demo_window);
        }
    }

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(4);
}

INTERNAL_FUNC void render_titlebar_text() {
    if (!titlebar_state.title_text) {
        return;
    }

    const UI_Theme_Palette& palette = get_current_palette();

    // Calculate text size for centering
    ImVec2 text_size = ImGui::CalcTextSize(titlebar_state.title_text);
    f32 titlebar_width = titlebar_state.titlebar_max.x - titlebar_state.titlebar_min.x;

    // Absolute center positioning (ignoring other elements)
    f32 text_x = titlebar_state.titlebar_min.x + (titlebar_width - text_size.x) * 0.5f;

    // Check for overlap with menu entries (if shown)
    if (titlebar_state.show_menus) {
        // Calculate menu area bounds
        f32 logo_size = 32.0f;
        f32 logo_margin = 8.0f;
        f32 menu_start_x = titlebar_state.titlebar_min.x + logo_margin + logo_size + 12.0f;

        // Estimate total menu width (File, View, Help with padding)
        f32 file_width = ImGui::CalcTextSize("File").x + 16; // text + padding
        f32 view_width = ImGui::CalcTextSize("View").x + 16; // text + padding
        f32 help_width = ImGui::CalcTextSize("Help").x + 16; // text + padding
        f32 menu_end_x = menu_start_x + file_width + view_width + help_width;

        // Calculate title bounds
        f32 title_start_x = text_x;
        f32 title_end_x = text_x + text_size.x;

        // Check if title overlaps with menu area
        b8 overlaps = (title_start_x < menu_end_x) && (title_end_x > menu_start_x);

        if (overlaps) {
            // Hide title when it would overlap with menus
            return;
        }
    }

    // Fixed distance from top edge
    f32 text_top_padding = 6.0f;
    f32 text_y = titlebar_state.titlebar_min.y + text_top_padding;

    ImVec2 text_pos = ImVec2(text_x, text_y);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddText(text_pos, palette.text, titlebar_state.title_text);
}

INTERNAL_FUNC void render_titlebar_buttons() {
    // Fixed button size (not scaling with titlebar height)
    f32 button_size = 26.0f; // Fixed size for consistent window controls
    f32 button_spacing = 2.0f;
    f32 right_margin = 4.0f;
    f32 top_padding = 2.0f; // Fixed distance from top edge

    f32 current_x = titlebar_state.titlebar_max.x - right_margin;

    // Close button (rightmost) - always show
    {
        current_x -= button_size;
        ImVec2 close_pos =
            ImVec2(current_x, titlebar_state.titlebar_min.y + top_padding); // Top-aligned
        ImVec2 close_size = ImVec2(button_size, button_size);

        const Vulkan_Image* close_icon =
            titlebar_state.icons_loaded ? &titlebar_state.close_icon : nullptr;
        if (render_titlebar_image_button(close_icon,
                "×",
                close_pos,
                close_size)) {
            CORE_DEBUG("Close button clicked - requesting application exit");
            platform_close_window();
        }
        current_x -= button_spacing;
    }

    // Maximize button - always show
    {
        current_x -= button_size;
        ImVec2 max_pos =
            ImVec2(current_x, titlebar_state.titlebar_min.y + top_padding); // Top-aligned
        ImVec2 max_size = ImVec2(button_size, button_size);

        // Check if window is maximized and use appropriate icon
        b8 is_maximized = platform_is_window_maximized();
        const Vulkan_Image* max_icon =
            is_maximized
                ? (titlebar_state.icons_loaded ? &titlebar_state.restore_icon
                                               : nullptr)
                : (titlebar_state.icons_loaded ? &titlebar_state.maximize_icon
                                               : nullptr);
        const char* fallback_text = is_maximized ? "🗗" : "□";

        if (render_titlebar_image_button(max_icon,
                fallback_text,
                max_pos,
                max_size)) {
            if (is_maximized) {
                CORE_DEBUG("Restore button clicked - restoring window");
                platform_restore_window();
            } else {
                CORE_DEBUG("Maximize button clicked - maximizing window");
                platform_maximize_window();
            }
        }
        current_x -= button_spacing;
    }

    // Minimize button - always show
    {
        current_x -= button_size;
        ImVec2 min_pos =
            ImVec2(current_x, titlebar_state.titlebar_min.y + top_padding); // Top-aligned

        ImVec2 min_size = ImVec2(button_size, button_size);

        const Vulkan_Image* min_icon = titlebar_state.icons_loaded
                                           ? &titlebar_state.minimize_icon
                                           : nullptr;

        if (render_titlebar_image_button(min_icon, "−", min_pos, min_size)) {

            CORE_DEBUG("Minimize button clicked - minimizing window");
            platform_minimize_window();
        }
    }
}

INTERNAL_FUNC b8 render_titlebar_button(const char* icon,
    ImVec2 pos,
    ImVec2 size) {

    const UI_Theme_Palette& palette = get_current_palette();
    ImGui::SetCursorScreenPos(pos);

    // Create invisible button for interaction
    ImGui::PushID(icon);
    b8 clicked = ImGui::InvisibleButton("##titlebar_btn", size);

    // Get button state for coloring
    u32 button_color = palette.titlebar;

    if (ImGui::IsItemActive()) {
        button_color = palette.highlight;
    } else if (ImGui::IsItemHovered()) {
        button_color = palette.button_hovered;
    }

    // Draw button background
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(pos,
        ImVec2(pos.x + size.x, pos.y + size.y),
        button_color);

    // Draw icon text centered
    ImVec2 text_size = ImGui::CalcTextSize(icon);
    ImVec2 text_pos = ImVec2(pos.x + (size.x - text_size.x) * 0.5f,
        pos.y + (size.y - text_size.y) * 0.5f);
    draw_list->AddText(text_pos, palette.text, icon);

    ImGui::PopID();
    return clicked;
}

INTERNAL_FUNC b8 render_titlebar_image_button(const Vulkan_Image* image,
    const char* fallback_text,
    ImVec2 pos,
    ImVec2 size) {
    const UI_Theme_Palette& palette = get_current_palette();
    ImGui::SetCursorScreenPos(pos);

    // Create invisible button for interaction
    ImGui::PushID(fallback_text);
    b8 clicked = ImGui::InvisibleButton("##titlebar_img_btn", size);

    // Get button state for coloring
    u32 button_color = palette.titlebar;
    if (ImGui::IsItemActive()) {
        button_color = palette.highlight;
    } else if (ImGui::IsItemHovered()) {
        button_color = palette.button_hovered;
    }

    // Draw button background
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(pos,
        ImVec2(pos.x + size.x, pos.y + size.y),
        button_color);

    if (image && titlebar_state.icons_loaded &&
        image->descriptor_set != VK_NULL_HANDLE) {
        // Draw the icon image centered in the button with aspect ratio
        // preservation Get original icon dimensions
        f32 original_width = (f32)image->width;
        f32 original_height = (f32)image->height;
        f32 original_aspect = original_width / original_height;

        // Calculate available space for icon (with padding)
        f32 available_width = size.x * 0.7f;  // 70% of button width for icon
        f32 available_height = size.y * 0.7f; // 70% of button height for icon

        // Use icons at their natural size (no scaling unless they're too large)
        f32 scale = 1.0f; // Start with natural size

        // Only scale down if the icon is larger than available space
        if (original_width > available_width) {
            scale = available_width / original_width;
        }
        if (original_height > available_height) {
            f32 height_scale = available_height / original_height;
            if (height_scale < scale) {
                scale = height_scale;
            }
        }

        // Calculate final dimensions (natural size or scaled down if needed)
        f32 scaled_width = original_width * scale;
        f32 scaled_height = original_height * scale;

        ImVec2 image_size = ImVec2(scaled_width, scaled_height);
        ImVec2 image_pos = ImVec2(pos.x + (size.x - image_size.x) *
                                              0.5f, // Center horizontally
            pos.y + size.y - image_size.y -
                (size.y * 0.15f)); // Anchor to bottom with padding

        // Render the icon with proper color tinting based on button state
        u32 icon_color = palette.text;
        if (ImGui::IsItemActive()) {
            icon_color = palette.text; // Keep same color when active
        } else if (ImGui::IsItemHovered()) {
            icon_color = IM_COL32_WHITE; // Brighter when hovered
        }

        draw_list->AddImage((ImTextureID)(intptr_t)image->descriptor_set,
            image_pos,
            ImVec2(image_pos.x + image_size.x, image_pos.y + image_size.y),
            ImVec2(0, 0),
            ImVec2(1, 1),
            icon_color);
    } else {
        // Fall back to text if no image is available
        // Debug: Log why we're falling back
        static b8 button_debug_logged = false;
        if (!button_debug_logged) {
            CORE_DEBUG(
                "Button fallback: image=%p, icons_loaded=%s, descriptor_set=%p",
                (void*)image,
                titlebar_state.icons_loaded ? "true" : "false",
                image ? (void*)image->descriptor_set : nullptr);
            button_debug_logged = true;
        }

        ImVec2 text_size = ImGui::CalcTextSize(fallback_text);
        ImVec2 text_pos = ImVec2(pos.x + (size.x - text_size.x) * 0.5f,
            pos.y + (size.y - text_size.y) * 0.5f);
        draw_list->AddText(text_pos, palette.text, fallback_text);
    }

    ImGui::PopID();
    return clicked;
}

INTERNAL_FUNC void render_titlebar_gradient() {
    // Get current theme palette for gradient colors
    extern UI_Theme ui_get_current_theme(); // Internal function from ui.cpp
    UI_Theme current_theme = ui_get_current_theme();
    const UI_Theme_Palette& palette = ui_themes_get_palette(current_theme);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Calculate gradient area (left 1/4 of titlebar)
    f32 titlebar_width = titlebar_state.titlebar_max.x - titlebar_state.titlebar_min.x;
    f32 gradient_width = titlebar_width * 0.25f; // 1/4 of width

    ImVec2 gradient_min = titlebar_state.titlebar_min;
    ImVec2 gradient_max = ImVec2(titlebar_state.titlebar_min.x + gradient_width,
                                titlebar_state.titlebar_max.y);

    // Choose gradient colors based on current theme
    u32 gradient_start_color, gradient_end_color;

    if (current_theme == UI_Theme::DARK) {
        // Orange to transparent gradient for dark theme (complements Prometheus branding)
        gradient_start_color = IM_COL32(236, 158, 36, 80); // Orange accent with transparency
        gradient_end_color = IM_COL32(236, 158, 36, 0);    // Fully transparent
    } else {
        // Purple to transparent gradient for Catppuccin theme
        gradient_start_color = IM_COL32(203, 166, 247, 60); // Mauve with transparency
        gradient_end_color = IM_COL32(203, 166, 247, 0);    // Fully transparent
    }

    // Render horizontal gradient
    draw_list->AddRectFilledMultiColor(
        gradient_min,
        gradient_max,
        gradient_start_color, // Top-left
        gradient_end_color,   // Top-right
        gradient_end_color,   // Bottom-right
        gradient_start_color  // Bottom-left
    );
}
