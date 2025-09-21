#include "client_ui.hpp"

// Core UI system
#include "ui/ui.hpp"
// Direct ImGui access (now available as public dependency from core)
#include "imgui.h"
#include "core/logger.hpp"
#include "defines.hpp"
#include "ui/ui_dockspace.hpp"

// Client UI state
struct Client_UI_State {
    b8 is_initialized;

    // Component state
    f32 slider_value;
    s32 counter;
    ImVec4 clear_color;

    // Window visibility states
    b8 show_prometheus_window;
    b8 show_demo_window;
};

// Zero memory initialization
internal_variable Client_UI_State client_state = {};

b8 client_ui_initialize() {
    CORE_DEBUG("Initializing client UI system...");

    if (client_state.is_initialized) {
        CORE_WARN("Client UI already initialized");
        return true;
    }

    // Zero out state and set defaults
    memory_zero(&client_state, sizeof(Client_UI_State));
    client_state.slider_value = 0.0f;
    client_state.counter = 0;
    client_state.clear_color = {0.45f, 0.55f, 0.60f, 1.00f};
    client_state.show_prometheus_window = true;
    client_state.show_demo_window = false;
    client_state.is_initialized = true;

    // Register components directly with core
    UI_Component prometheus_window = {
        .name = "prometheus_window",
        .on_render = client_ui_render_prometheus_window,
        .on_attach = nullptr,
        .on_detach = nullptr,
        .user_data = &client_state,
        .is_active = true
    };

    if (!ui_register_component(&prometheus_window)) {
        CORE_ERROR("Failed to register prometheus window component");
        return false;
    }

    // Register menu callback directly
    ui_register_menu_callback(client_ui_render_menus, &client_state);

    CORE_INFO("Client UI system initialized successfully");
    return true;
}

void client_ui_shutdown() {
    CORE_DEBUG("Shutting down client UI system...");

    if (!client_state.is_initialized) {
        CORE_WARN("Client UI not initialized");
        return;
    }

    // Unregister components
    ui_unregister_component("prometheus_window");

    client_state.is_initialized = false;
    CORE_DEBUG("Client UI system shut down successfully");
}

// Component implementations (migrated from core/src/ui/ui_components.cpp)

void client_ui_render_prometheus_window(void* user_data) {
    Client_UI_State* state = (Client_UI_State*)user_data;

    if (!state->show_prometheus_window) {
        return;
    }

    ImGui::Begin("Prometheus Engine", &state->show_prometheus_window);

    // Engine info section
    ImGui::SeparatorText("Engine Information");
    ImGui::Text("Prometheus Game Engine");
    ImGui::Text("Version: 1.0.0-dev");
    ImGui::Text("Architecture: Vulkan + ImGui + SDL3");

    ImGui::Spacing();

    // UI Controls section
    ImGui::SeparatorText("UI Controls");
    // Note: Performance window is controlled by core, not client
    // Could add a callback to core to toggle it if needed

    ImGui::Spacing();

    // Interactive controls section
    ImGui::SeparatorText("Interactive Controls");
    ImGui::SliderFloat("Test Slider", &state->slider_value, 0.0f, 1.0f);
    ImGui::ColorEdit3("Clear Color", (float*)&state->clear_color);

    if (ImGui::Button("Test Button")) {
        state->counter++;
        CORE_INFO("Button clicked! Count: %d", state->counter);
    }

    ImGui::SameLine();
    ImGui::Text("Clicks: %d", state->counter);

    ImGui::Spacing();

    // System info section
    ImGui::SeparatorText("System Information");
    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::Text("Vertices: %d", io.MetricsRenderVertices);
    ImGui::Text("Indices: %d", io.MetricsRenderIndices);

    ImGui::End();
}

// Menu system implementation (migrated from core/src/ui/ui_components.cpp)
void client_ui_render_menus(void* user_data) {
    Client_UI_State* state = (Client_UI_State*)user_data;

    // File menu
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
            CORE_DEBUG("File -> New selected");
        }
        if (ImGui::MenuItem("Open", "Ctrl+O")) {
            CORE_DEBUG("File -> Open selected");
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            CORE_DEBUG("File -> Save selected");
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit", "Alt+F4")) {
            CORE_DEBUG("File -> Exit selected");
            // TODO: Request application exit
        }
        ImGui::EndMenu();
    }

    // View menu
    if (ImGui::BeginMenu("View")) {
        ImGui::MenuItem("Prometheus Window", nullptr, &state->show_prometheus_window);
        ImGui::MenuItem("Demo Window", nullptr, &state->show_demo_window);
        ImGui::Separator();
        if (ImGui::MenuItem("Reset Layout")) {
            CORE_DEBUG("View -> Reset Layout selected");
            ui_dockspace_reset_layout();
        }

        ImGui::EndMenu();
    }

    // Help menu
    if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("About")) {
            CORE_DEBUG("Help -> About selected");
            // TODO: Show about dialog
        }
        ImGui::EndMenu();
    }

    // Show demo window if enabled
    if (state->show_demo_window) {
        ImGui::ShowDemoWindow(&state->show_demo_window);
    }
}
