#include "ui.hpp"
#include "ui_components.hpp"

#include "core/logger.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "renderer/renderer_platform.hpp"

// Internal UI state
internal_variable UI_State ui_state = {};
internal_variable UI_EventCallback event_callback = nullptr;

// Forward declarations for internal functions
INTERNAL_FUNC b8 setup_imgui_context(f32 main_scale);
INTERNAL_FUNC void setup_imgui_style();

b8 ui_initialize() {
    CORE_DEBUG("Initializing UI subsystem...");

    if (ui_state.is_initialized) {
        CORE_WARN("UI subsystem already initialized");
        return true;
    }

    // Get window details from platform
    u32 width, height;
    f32 main_scale;
    if (!platform_get_window_details(&width, &height, &main_scale)) {
        CORE_ERROR("Failed to get window details for UI initialization");
        return false;
    }

    // Setup ImGui context
    if (!setup_imgui_context(main_scale)) {
        CORE_ERROR("Failed to setup ImGui context");
        return false;
    }

    // Initialize default UI state
    ui_state.show_demo_window = true;
    ui_state.show_simple_window = true;
    ui_state.is_initialized = true;

    CORE_INFO("UI subsystem initialized successfully");
    return true;
}

void ui_shutdown() {
    CORE_DEBUG("Shutting down UI subsystem...");

    if (!ui_state.is_initialized) {
        CORE_WARN("UI subsystem not initialized");
        return;
    }

    // Wait for renderer to finish all operations before shutting down ImGui
    // This prevents Vulkan validation errors when destroying resources in use
    if (!renderer_wait_idle()) {
        CORE_WARN("Failed to wait for renderer idle during UI shutdown");
    }

    // Shutdown ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // Reset state
    ui_state = {};
    event_callback = nullptr;

    CORE_DEBUG("UI subsystem shut down successfully");
}

b8 ui_process_event(const SDL_Event* event) {
    if (!ui_state.is_initialized) {
        return false;
    }

    // Let ImGui process the event
    ImGui_ImplSDL3_ProcessEvent(event);

    // Don't consume ESC key - let platform handle it for quit functionality
    if (event->type == SDL_EVENT_KEY_DOWN && event->key.key == SDLK_ESCAPE) {
        return false;
    }

    // Check if ImGui wants to handle this event (e.g., when input fields are focused)
    ImGuiIO& io = ImGui::GetIO();

    // Let platform handle events when UI doesn't need them
    switch (event->type) {
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            return io.WantCaptureKeyboard;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_WHEEL:
        case SDL_EVENT_MOUSE_MOTION:
            return io.WantCaptureMouse;
        default:
            return false;
    }
}

void ui_new_frame() {
    if (!ui_state.is_initialized) {
        return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
}

ImDrawData* ui_render() {
    if (!ui_state.is_initialized) {
        return nullptr;
    }

    // Render all UI components using the component system
    ui_render_all_components(&ui_state);

    // Finalize rendering and get draw data
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    // Check if minimized
    const bool is_minimized =
        (draw_data->DisplaySize.x <= 0.0f ||
         draw_data->DisplaySize.y <= 0.0f);

    return is_minimized ? nullptr : draw_data;
}

const UI_State* ui_get_state() {
    return &ui_state;
}

void ui_set_event_callback(UI_EventCallback callback) {
    event_callback = callback;
}

// Internal function implementations
INTERNAL_FUNC b8 setup_imgui_context(f32 main_scale) {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Configure ImGui
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup style
    setup_imgui_style();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // Initialize platform backend for ImGui
    u32 width, height;
    f32 scale;
    platform_get_window_details(&width, &height, &scale);

    // Get the SDL window through renderer interface to maintain clean separation
    SDL_Window* window = (SDL_Window*)renderer_get_sdl_window();
    if (!window) {
        CORE_ERROR("SDL window not available for UI initialization");
        return false;
    }

    ImGui_ImplSDL3_InitForVulkan(window);

    // Initialize Vulkan backend for ImGui through renderer
    if (!renderer_init_imgui_vulkan()) {
        CORE_ERROR("Failed to initialize ImGui Vulkan backend");
        return false;
    }

    CORE_DEBUG("ImGui context setup completed");
    return true;
}

INTERNAL_FUNC void setup_imgui_style() {
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight(); // Alternative style

    // Future: Custom Prometheus theme can be added here
}
