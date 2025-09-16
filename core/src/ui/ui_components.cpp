#include "ui_components.hpp"
#include "ui.hpp"

#include "imgui.h"
#include "core/logger.hpp"

// Internal component state (static variables for UI components)
struct Component_State {
    // Simple window state
    float slider_value;
    int counter;
    ImVec4 clear_color;

    // Performance window state
    b8 show_performance;

    // Future components can add their state here
};

internal_variable Component_State comp_state = {
    .slider_value = 0.0f,
    .counter = 0,
    .clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f),
    .show_performance = false
};

void ui_render_demo_window(UI_State* ui_state) {
    if (!ui_state->show_demo_window) {
        return;
    }

    ImGui::ShowDemoWindow(&ui_state->show_demo_window);
}

void ui_render_prometheus_window(UI_State* ui_state) {
    if (!ui_state->show_simple_window) {
        return;
    }

    ImGui::Begin("Prometheus Engine", &ui_state->show_simple_window);

    // Engine info section
    ImGui::SeparatorText("Engine Information");
    ImGui::Text("Prometheus Game Engine");
    ImGui::Text("Version: 1.0.0-dev");
    ImGui::Text("Architecture: Vulkan + ImGui + SDL3");

    ImGui::Spacing();

    // UI Controls section
    ImGui::SeparatorText("UI Controls");
    ImGui::Checkbox("Show Demo Window", &ui_state->show_demo_window);
    ImGui::Checkbox("Show Performance Window", &comp_state.show_performance);

    ImGui::Spacing();

    // Interactive controls section
    ImGui::SeparatorText("Interactive Controls");
    ImGui::SliderFloat("Test Slider", &comp_state.slider_value, 0.0f, 1.0f);
    ImGui::ColorEdit3("Clear Color", (float*)&comp_state.clear_color);

    if (ImGui::Button("Test Button")) {
        comp_state.counter++;
        CORE_INFO("Button clicked! Count: %d", comp_state.counter);
    }

    ImGui::SameLine();
    ImGui::Text("Clicks: %d", comp_state.counter);

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

void ui_render_performance_window(UI_State* ui_state) {
    if (!comp_state.show_performance) {
        return;
    }

    ImGui::Begin("Performance Metrics", &comp_state.show_performance);

    ImGuiIO& io = ImGui::GetIO();

    // Frame timing
    ImGui::SeparatorText("Frame Timing");
    ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
    ImGui::Text("FPS: %.1f", io.Framerate);

    // Memory info (placeholder - would need actual memory tracking)
    ImGui::SeparatorText("Memory Usage");
    ImGui::Text("Memory tracking not implemented yet");

    // Render stats
    ImGui::SeparatorText("Render Statistics");
    ImGui::Text("Draw Calls: %d", io.MetricsRenderWindows);
    ImGui::Text("Vertices: %d", io.MetricsRenderVertices);
    ImGui::Text("Indices: %d", io.MetricsRenderIndices);

    // Platform info
    ImGui::SeparatorText("Platform Information");
    ImGui::Text("Backend: %s", io.BackendPlatformName ? io.BackendPlatformName : "Unknown");
    ImGui::Text("Renderer: %s", io.BackendRendererName ? io.BackendRendererName : "Unknown");

    ImGui::End();
}

void ui_render_all_components(UI_State* ui_state) {
    if (!ui_state || !ui_state->is_initialized) {
        return;
    }

    // Render all active UI components
    ui_render_demo_window(ui_state);
    ui_render_prometheus_window(ui_state);
    ui_render_performance_window(ui_state);

    // Future components can be added here:
    // ui_render_scene_hierarchy(ui_state);
    // ui_render_asset_browser(ui_state);
    // ui_render_console(ui_state);
}