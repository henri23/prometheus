#include "ui_dockspace.hpp"
#include "ui_titlebar.hpp"

#include "core/logger.hpp"
#include "imgui.h"

// Internal dockspace state
struct Dockspace_State {
    b8 is_initialized;
    unsigned int dockspace_id;
    b8 dockspace_open;
    b8 window_began; // Track if ImGui::Begin() was called this frame
};

internal_variable Dockspace_State dockspace_state = {};

b8 ui_dockspace_initialize() {
    CORE_DEBUG("Initializing dockspace system...");

    if (dockspace_state.is_initialized) {
        CORE_WARN("Dockspace already initialized");
        return true;
    }

    // Setup ImGui docking configuration
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    CORE_DEBUG("ImGui docking enabled");

    // Initialize state (defer ID generation until first render)
    dockspace_state.dockspace_id = 0; // Will be set on first render
    dockspace_state.dockspace_open = true;
    dockspace_state.is_initialized = true;

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

    if (!dockspace_state.is_initialized) {
        return;
    }

    UI_State* ui_state = (UI_State*)user_data;
    if (!ui_state) {
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
    f32 titlebar_height = TITLEBAR_HEIGHT;
    work_pos.y += titlebar_height;
    work_size.y -= titlebar_height;

    // Set up the main dockspace window
    ImGui::SetNextWindowPos(work_pos);
    ImGui::SetNextWindowSize(work_size);
    ImGui::SetNextWindowViewport(viewport->ID);

    // Configure window for fullscreen dockspace
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // Setup window flags for dockspace
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    // Begin the main dockspace window
    const char* window_name = "DockSpace";
    ImGui::Begin(window_name, &dockspace_state.dockspace_open, window_flags);
    dockspace_state.window_began = true;

    // Pop style vars
    ImGui::PopStyleVar(3);

    // Create the dockspace
    ImGuiIO& io = ImGui::GetIO();
    // CORE_DEBUG("Docking enabled: %s", (io.ConfigFlags &
    // ImGuiConfigFlags_DockingEnable) ? "YES" : "NO");

    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        // Set minimum window size for better docking experience
        ImGuiStyle& style = ImGui::GetStyle();
        float minWinSizeX = style.WindowMinSize.x;
        style.WindowMinSize.x = 300.0f;

        // Create dockspace with simpler approach (like Walnut)
        // CORE_DEBUG("Creating dockspace with ID: %u",
        // dockspace_state.dockspace_id);
        ImGui::DockSpace(dockspace_state.dockspace_id);

        // Restore original window size
        style.WindowMinSize.x = minWinSizeX;

        // Let ImGui handle dockspace layout automatically
    } else {
        CORE_ERROR("ImGui docking is not enabled!");
    }

    // Menu rendering is handled by the titlebar
}

void ui_dockspace_end() {
    if (!dockspace_state.is_initialized) {
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
