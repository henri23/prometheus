#include "ui_dockspace.hpp"
#include "ui.hpp"
#include "ui_titlebar.hpp"

#include "imgui.h"
#include "imgui_internal.h"
#include "core/logger.hpp"
#include "core/asserts.hpp"

// Internal dockspace state
struct Dockspace_State {
    b8 is_initialized;
    b8 is_enabled;
    UI_Dockspace_Config config;
    unsigned int dockspace_id;
    b8 dockspace_open;
    b8 first_frame;
    b8 window_began; // Track if ImGui::Begin() was called this frame
};

internal_variable Dockspace_State dockspace_state = {};

// Default dockspace configuration
internal_variable UI_Dockspace_Config default_config = {
    .enable_docking = true,
    .enable_viewports = false,  // Disable viewports for stability
    .show_menubar = true,
    .fullscreen_dockspace = true,
    .no_titlebar = true,
    .no_collapse = true,
    .no_resize = true,
    .no_move = true,
    .no_background = false,
    .menubar_height = 0.0f,
    .window_flags = 0,
    .dockspace_flags = 0
};

// Internal functions
INTERNAL_FUNC void setup_dockspace_window_flags();
INTERNAL_FUNC void setup_dockspace_flags();
INTERNAL_FUNC void render_main_menubar();
INTERNAL_FUNC void configure_initial_layout();

b8 ui_dockspace_initialize(const UI_Dockspace_Config* config) {
    CORE_DEBUG("Initializing dockspace system...");

    if (dockspace_state.is_initialized) {
        CORE_WARN("Dockspace already initialized");
        return true;
    }

    // Use provided config or default
    if (config) {
        dockspace_state.config = *config;
    } else {
        dockspace_state.config = default_config;
    }

    // Setup ImGui docking configuration
    ImGuiIO& io = ImGui::GetIO();
    if (dockspace_state.config.enable_docking) {
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        CORE_DEBUG("ImGui docking enabled");
    }

    if (dockspace_state.config.enable_viewports) {
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        CORE_DEBUG("ImGui viewports enabled");
    }

    // Initialize state (defer ID generation until first render)
    dockspace_state.dockspace_id = 0; // Will be set on first render
    dockspace_state.dockspace_open = true;
    dockspace_state.first_frame = true;
    dockspace_state.is_enabled = true;
    dockspace_state.is_initialized = true;

    // Setup window and dockspace flags
    setup_dockspace_window_flags();
    setup_dockspace_flags();

    CORE_INFO("Dockspace system initialized successfully");
    return true;
}

void ui_dockspace_shutdown() {
    CORE_DEBUG("Shutting down dockspace system...");

    if (!dockspace_state.is_initialized) {
        CORE_WARN("Dockspace not initialized");
        return;
    }

    // Reset state
    dockspace_state = {};

    CORE_DEBUG("Dockspace system shut down successfully");
}

void ui_dockspace_begin(void* user_data) {
    // Reset the window_began flag at the start of each frame
    dockspace_state.window_began = false;

    if (!dockspace_state.is_initialized || !dockspace_state.is_enabled) {
        return;
    }

    UI_State* ui_state = (UI_State*)user_data;
    if (!ui_state || !ui_state->show_dockspace) {
        dockspace_state.window_began = false;
        return;
    }

    // Generate dockspace ID on first use (when ImGui context is ready)
    if (dockspace_state.dockspace_id == 0) {
        dockspace_state.dockspace_id = ImGui::GetID("MainDockspace");
        CORE_DEBUG("Generated dockspace ID: %u", dockspace_state.dockspace_id);
    }

    // Setup viewport for fullscreen dockspace
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_pos = viewport->WorkPos;
    ImVec2 work_size = viewport->WorkSize;

    // Adjust for custom titlebar if enabled
    if (ui_state->custom_titlebar_enabled) {
        f32 titlebar_height = ui_titlebar_get_height();
        work_pos.y += titlebar_height;
        work_size.y -= titlebar_height;
    }

    // Set up the main dockspace window
    ImGui::SetNextWindowPos(work_pos);
    ImGui::SetNextWindowSize(work_size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Configure window for fullscreen dockspace
    if (dockspace_state.config.fullscreen_dockspace) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    }

    // Begin the main dockspace window
    const char* window_name = "DockSpace";
    ImGui::Begin(window_name, &dockspace_state.dockspace_open, dockspace_state.config.window_flags);
    dockspace_state.window_began = true;

    // Pop style vars if fullscreen
    if (dockspace_state.config.fullscreen_dockspace) {
        ImGui::PopStyleVar(3);
    }

    // Create the dockspace
    ImGuiIO& io = ImGui::GetIO();
    // CORE_DEBUG("Docking enabled: %s", (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) ? "YES" : "NO");

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        // Set minimum window size for better docking experience
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 300.0f;

        // Create dockspace with simpler approach (like Walnut)
        // CORE_DEBUG("Creating dockspace with ID: %u", dockspace_state.dockspace_id);
        ImGui::DockSpace(dockspace_state.dockspace_id);

        // Restore original window size
        style.WindowMinSize.x = minWinSizeX;

        // Configure initial layout on first frame
        if (dockspace_state.first_frame) {
            CORE_DEBUG("Configuring initial dockspace layout");
            configure_initial_layout();
            dockspace_state.first_frame = false;
        }
    } else {
        CORE_ERROR("ImGui docking is not enabled!");
    }

    // Render main menubar if enabled
    if (dockspace_state.config.show_menubar) {
        render_main_menubar();
    }
}

void ui_dockspace_end() {
    if (!dockspace_state.is_initialized || !dockspace_state.is_enabled) {
        return;
    }

    // Only call ImGui::End() if we actually called ImGui::Begin()
    if (dockspace_state.window_began) {
        ImGui::End(); // End DockSpace window
        dockspace_state.window_began = false;
    }
}

void ui_dockspace_render(void* user_data) {
    // The dockspace itself doesn't render content
    // It's a container that manages other windows
    // This function is here for component system compatibility
    ui_dockspace_begin(user_data);
    ui_dockspace_end();
}

b8 ui_dockspace_is_enabled() {
    return dockspace_state.is_initialized && dockspace_state.is_enabled;
}

void ui_dockspace_set_enabled(b8 enabled) {
    if (dockspace_state.is_initialized) {
        dockspace_state.is_enabled = enabled;
        CORE_DEBUG("Dockspace %s", enabled ? "enabled" : "disabled");
    }
}

unsigned int ui_dockspace_get_id() {
    return dockspace_state.is_initialized ? dockspace_state.dockspace_id : 0;
}

void ui_dockspace_reset_layout() {
    if (dockspace_state.is_initialized) {
        dockspace_state.first_frame = true;
        CORE_DEBUG("Dockspace layout reset requested");
    }
}

// Internal function implementations
INTERNAL_FUNC void setup_dockspace_window_flags() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    if (dockspace_state.config.fullscreen_dockspace) {
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                       ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }

    if (dockspace_state.config.no_background) {
        window_flags |= ImGuiWindowFlags_NoBackground;
    }

    dockspace_state.config.window_flags = window_flags;
}

INTERNAL_FUNC void setup_dockspace_flags() {
    // Use default dockspace flags for maximum compatibility
    ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // Don't add any special flags - let ImGui handle docking naturally
    dockspace_state.config.dockspace_flags = dockspace_flags;
}

INTERNAL_FUNC void render_main_menubar() {
    if (ImGui::BeginMenuBar()) {
        // Call client-defined menu callback
        const UI_State* ui_state = ui_get_state();
        if (ui_state && ui_state->menu_callback) {
            ui_state->menu_callback(ui_state->menu_user_data);
        }
        // Note: No fallback to core menu system - menus are now client responsibility

        ImGui::EndMenuBar();
    }
}

INTERNAL_FUNC void configure_initial_layout() {
    // Simplified initial layout configuration
    // Let ImGui handle most of the layout automatically

    ImGuiID dockspace_id = dockspace_state.dockspace_id;

    // Only set up basic dockspace structure, don't force specific window positions
    // This allows users to dock windows naturally where they want them

    CORE_DEBUG("Dockspace ready for user-defined layout");
}
