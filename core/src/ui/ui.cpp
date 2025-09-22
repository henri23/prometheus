#include "ui.hpp"
#include "ui_dockspace.hpp"
#include "ui_fonts.hpp"
#include "ui_themes.hpp"
#include "ui_titlebar.hpp"

#include "core/logger.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "renderer/renderer_platform.hpp"

// Internal UI state - not exposed to client
struct UI_State {
    Auto_Array<UI_Layer>* layers;
    UI_Theme current_theme;
	PFN_menu_callback menu_callback;
    b8 is_initialized;
};

internal_variable UI_State ui_state;

// Forward declarations for internal functions
INTERNAL_FUNC b8 setup_imgui_context(f32 main_scale);

b8 ui_initialize(
	UI_Theme theme, 
	Auto_Array<UI_Layer>* layers,
	PFN_menu_callback menu_callback,
	const char* app_name) {

    CORE_DEBUG("Initializing UI subsystem...");

    if (ui_state.is_initialized) {
        CORE_WARN("UI subsystem already initialized");
        return true;
    }

    // Zero out the state (though it should already be zero)
    memory_zero(&ui_state, sizeof(UI_State));

    // Set configuration from parameters BEFORE setting up ImGui context
    ui_state.current_theme = theme;

    // Get window details from platform
    u32 width, height;
    f32 main_scale;

    if (!platform_get_window_details(&width, &height, &main_scale)) {
        CORE_ERROR("Failed to get window details for UI initialization");
        return false;
    }

    // Setup ImGui context (now with correct theme in ui_state)
    if (!setup_imgui_context(main_scale)) {
        CORE_ERROR("Failed to setup ImGui context");
        return false;
    }

    // Set remaining configuration parameters
    ui_state.layers = layers;

    ui_state.is_initialized = true;

    // Initialize font system
    if (!ui_fonts_initialize()) {
        CORE_ERROR("Failed to initialize font system");
        return false;
    }

    // Load default fonts
    ui_fonts_register_defaults();

    // Initialize infrastructure components
    ui_dockspace_initialize();
    ui_titlebar_initialize(menu_callback, app_name); // No config needed

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

    // Shutdown infrastructure components
    ui_titlebar_shutdown();
    CORE_DEBUG("UI titlebar shutdown complete.");

    ui_dockspace_shutdown();
    CORE_DEBUG("UI dockspace shutdown complete.");

    ui_fonts_shutdown();
    CORE_DEBUG("UI fonts shutdown complete.");

    // Shutdown ImGui
    ImGui_ImplVulkan_Shutdown();
    CORE_DEBUG("ImGui Vulkan shutdown complete.");

    ImGui_ImplSDL3_Shutdown();
    CORE_DEBUG("ImGui SDL3 shutdown complete.");

    ImGui::DestroyContext();
    CORE_DEBUG("ImGui context destroyed.");

    // Cleanup component system
    if (!ui_state.layers->empty()) {
        // Call detach callbacks for all active components
        for (u32 i = 0; i < ui_state.layers->size(); ++i) {
            UI_Layer* component = &ui_state.layers->data[i];
            if (component->on_detach) {

                component->on_detach(component->component_state);
                CORE_DEBUG("Component %u detach callback complete", i);
            }
        }

        ui_state.layers->clear();
        CORE_DEBUG("UI components cleared.");
    }

    // Reset state
    ui_state.is_initialized = false;

    ui_state.current_theme = UI_Theme::DARK;
    ui_state.menu_callback = nullptr;

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

    // Check if ImGui wants to handle this event (e.g., when input fields are
    // focused)
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

void ui_begin_frame() {
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
    // Render dockspace first - it provides the container for other windows
    ui_dockspace_begin(&ui_state);

    // Render custom titlebar if enabled
    ui_titlebar_render();

    // Render all registered UI components
    for (u32 i = 0; i < ui_state.layers->length; ++i) {
        UI_Layer* layer = &ui_state.layers->data[i];

        if (layer->on_render)
            layer->on_render(layer->component_state);
    }

    // End dockspace if it was started
    ui_dockspace_end();

    // Finalize rendering and get draw data
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

// Update and Render additional Platform Windows (for
// ImGuiConfigFlags_ViewportsEnable) Only render viewports if they're enabled
// and working properly
#ifdef PROMETHEUS_ENABLE_VIEWPORTS
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
#endif

    // Check if minimized
    const bool is_minimized =
        (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

    return is_minimized ? nullptr : draw_data;
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

// SDL3 viewport support is experimental and can cause crashes
// Only enable if explicitly requested and working properly
#ifdef PROMETHEUS_ENABLE_VIEWPORTS
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport
                                                        // / Platform Windows
    CORE_DEBUG("ImGui viewports enabled (experimental with SDL3)");
#else
    CORE_DEBUG("ImGui viewports disabled (SDL3 compatibility mode)");
#endif

    // Setup style and theme using the theme system
    ImGuiStyle& style = ImGui::GetStyle();

    // Apply the configured theme
    ui_themes_apply(ui_state.current_theme, style);

    // Setup scaling
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

// When viewports are enabled we tweak WindowRounding/WindowBg so platform
// windows can look identical to regular ones.
#ifdef PROMETHEUS_ENABLE_VIEWPORTS
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
#endif

    // Initialize platform backend for ImGui
    u32 width, height;
    f32 scale;
    platform_get_window_details(&width, &height, &scale);

    // Get the SDL window through renderer interface to maintain clean
    // separation
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

// Component system implementation

// Internal accessor for current theme (for core UI components only)
UI_Theme ui_get_current_theme() { return ui_state.current_theme; }
