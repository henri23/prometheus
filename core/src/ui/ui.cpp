#include "ui.hpp"
#include "assets/assets.hpp"
#include "ui_config.hpp"
#include "ui_components.hpp"
#include "ui_dockspace.hpp"
#include "ui_titlebar.hpp"
#include "ui_fonts.hpp"

#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "platform/platform.hpp"
#include "renderer/renderer_backend.hpp"
#include "renderer/renderer_platform.hpp"

// Forward declaration for client UI types
typedef void (*Client_UI_RenderCallback)(void* user_data);
typedef void (*Client_UI_AttachCallback)(void* user_data);
typedef void (*Client_UI_DetachCallback)(void* user_data);
typedef void (*Client_MenuCallback)(void* user_data);

struct Client_UI_Component {
    const char* name;
    Client_UI_RenderCallback on_render;
    Client_UI_AttachCallback on_attach;
    Client_UI_DetachCallback on_detach;
    void* user_data;
    b8 is_active;
};

struct Client_UI_Config {
    Client_UI_Component* components;
    u32 component_count;
    Client_MenuCallback menu_callback;
    void* menu_user_data;
};

// Internal UI state
internal_variable UI_State ui_state = {};
internal_variable UI_EventCallback event_callback = nullptr;
internal_variable Client_UI_Config* client_config = nullptr;

// Forward declarations for internal functions
INTERNAL_FUNC b8 setup_imgui_context(f32 main_scale);

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
    ui_state.show_dockspace = true;
    ui_state.custom_titlebar_enabled = true;  // Enable custom titlebar
    ui_state.menu_callback = nullptr;
    ui_state.menu_user_data = nullptr;

    // Initialize component system
    ui_state.component_capacity = 16; // Start with 16 slots
    ui_state.component_count = 0;
    ui_state.components = (UI_Component*)memory_allocate(
        sizeof(UI_Component) * ui_state.component_capacity,
        Memory_Tag::UI);

    if (!ui_state.components) {
        CORE_ERROR("Failed to allocate memory for UI component system");
        return false;
    }

    ui_state.is_initialized = true;

    // Initialize font system
    if (!ui_fonts_initialize()) {
        CORE_ERROR("Failed to initialize font system");
        return false;
    }

    // Load default fonts
    ui_fonts_register_defaults();

    // Initialize infrastructure components
    ui_dockspace_initialize(nullptr); // Use default config
    ui_titlebar_initialize(nullptr);  // Use default config

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
    ui_dockspace_shutdown();
    ui_fonts_shutdown();

    // Shutdown ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    // Cleanup component system
    if (ui_state.components) {
        // Call detach callbacks for all active components
        for (u32 i = 0; i < ui_state.component_count; ++i) {
            UI_Component* component = &ui_state.components[i];
            if (component->on_detach) {
                component->on_detach(component->user_data);
            }
        }

        memory_deallocate(ui_state.components,
                         sizeof(UI_Component) * ui_state.component_capacity,
                         Memory_Tag::UI);
    }

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

    // Update and Render additional Platform Windows (for ImGuiConfigFlags_ViewportsEnable)
    // Only render viewports if they're enabled and working properly
    #ifdef PROMETHEUS_ENABLE_VIEWPORTS
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
    #endif

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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

    // SDL3 viewport support is experimental and can cause crashes
    // Only enable if explicitly requested and working properly
    #ifdef PROMETHEUS_ENABLE_VIEWPORTS
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    CORE_DEBUG("ImGui viewports enabled (experimental with SDL3)");
    #else
    CORE_DEBUG("ImGui viewports disabled (SDL3 compatibility mode)");
    #endif

    // Setup style and theme (inlined from ui_theme.cpp)
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Theme colors (Walnut-inspired dark theme)
    constexpr ImU32 accent            = IM_COL32(236, 158,  36, 255);
    constexpr ImU32 highlight         = IM_COL32( 39, 185, 242, 255);
    constexpr ImU32 nice_blue         = IM_COL32( 83, 232, 254, 255);
    constexpr ImU32 compliment        = IM_COL32( 78, 151, 166, 255);
    constexpr ImU32 background        = IM_COL32( 36,  36,  36, 255);
    constexpr ImU32 background_dark   = IM_COL32( 26,  26,  26, 255);
    constexpr ImU32 titlebar          = IM_COL32( 21,  21,  21, 255);
    constexpr ImU32 property_field    = IM_COL32( 15,  15,  15, 255);
    constexpr ImU32 text              = IM_COL32(192, 192, 192, 255);
    constexpr ImU32 text_brighter     = IM_COL32(210, 210, 210, 255);
    constexpr ImU32 text_darker       = IM_COL32(128, 128, 128, 255);
    constexpr ImU32 text_error        = IM_COL32(230,  51,  51, 255);
    constexpr ImU32 muted             = IM_COL32( 77,  77,  77, 255);
    constexpr ImU32 group_header      = IM_COL32( 47,  47,  47, 255);
    constexpr ImU32 selection         = IM_COL32(237, 192, 119, 255);
    constexpr ImU32 selection_muted   = IM_COL32(237, 201, 142,  23);
    constexpr ImU32 background_popup  = IM_COL32( 50,  50,  50, 255);

    // Apply theme colors
    colors[ImGuiCol_Header]         = ImGui::ColorConvertU32ToFloat4(group_header);
    colors[ImGuiCol_HeaderHovered]  = ImGui::ColorConvertU32ToFloat4(group_header);
    colors[ImGuiCol_HeaderActive]   = ImGui::ColorConvertU32ToFloat4(group_header);
    colors[ImGuiCol_Button]         = ImColor(56, 56, 56, 200);
    colors[ImGuiCol_ButtonHovered]  = ImColor(70, 70, 70, 255);
    colors[ImGuiCol_ButtonActive]   = ImColor(56, 56, 56, 150);
    colors[ImGuiCol_FrameBg]        = ImGui::ColorConvertU32ToFloat4(property_field);
    colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(property_field);
    colors[ImGuiCol_FrameBgActive]  = ImGui::ColorConvertU32ToFloat4(property_field);
    colors[ImGuiCol_Tab]                = ImGui::ColorConvertU32ToFloat4(titlebar);
    colors[ImGuiCol_TabHovered]         = ImColor(255, 225, 135, 30);
    colors[ImGuiCol_TabActive]          = ImColor(255, 225, 135, 60);
    colors[ImGuiCol_TabUnfocused]       = ImGui::ColorConvertU32ToFloat4(titlebar);
    colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];
    colors[ImGuiCol_TitleBg]         = ImGui::ColorConvertU32ToFloat4(titlebar);
    colors[ImGuiCol_TitleBgActive]   = ImGui::ColorConvertU32ToFloat4(titlebar);
    colors[ImGuiCol_TitleBgCollapsed]= ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    colors[ImGuiCol_ResizeGrip]        = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
    colors[ImGuiCol_ResizeGripActive]  = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
    colors[ImGuiCol_ScrollbarBg]        = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
    colors[ImGuiCol_ScrollbarGrab]      = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabHovered]= ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
    colors[ImGuiCol_ScrollbarGrabActive]= ImVec4(0.51f, 0.51f, 0.51f, 1.0f);
    colors[ImGuiCol_Text]            = ImGui::ColorConvertU32ToFloat4(text);
    colors[ImGuiCol_CheckMark]       = ImGui::ColorConvertU32ToFloat4(text);
    colors[ImGuiCol_Separator]        = ImGui::ColorConvertU32ToFloat4(background_dark);
    colors[ImGuiCol_SeparatorActive]  = ImGui::ColorConvertU32ToFloat4(highlight);
    colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);
    colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(titlebar);
    colors[ImGuiCol_ChildBg]  = ImGui::ColorConvertU32ToFloat4(background);
    colors[ImGuiCol_PopupBg]  = ImGui::ColorConvertU32ToFloat4(background_popup);
    colors[ImGuiCol_Border]   = ImGui::ColorConvertU32ToFloat4(background_dark);
    colors[ImGuiCol_TableHeaderBg]    = ImGui::ColorConvertU32ToFloat4(group_header);
    colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(background_dark);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Style tweaks
    style.FrameRounding  = 2.5f;
    style.FrameBorderSize= 1.0f;
    style.IndentSpacing  = 11.0f;

    // Setup scaling
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    #ifdef PROMETHEUS_ENABLE_VIEWPORTS
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    #endif

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

// Component system implementation

INTERNAL_FUNC UI_Component* find_component_by_name(const char* name) {
    RUNTIME_ASSERT_MSG(name, "Component name cannot be null");

    for (u32 i = 0; i < ui_state.component_count; ++i) {
        if (ui_state.components[i].name && strcmp(ui_state.components[i].name, name) == 0) {
            return &ui_state.components[i];
        }
    }
    return nullptr;
}

INTERNAL_FUNC b8 expand_component_registry() {
    u32 new_capacity = ui_state.component_capacity * 2;
    UI_Component* new_components = (UI_Component*)memory_allocate(
        sizeof(UI_Component) * new_capacity,
        Memory_Tag::UI);

    if (!new_components) {
        CORE_ERROR("Failed to expand component registry");
        return false;
    }

    // Copy existing components
    for (u32 i = 0; i < ui_state.component_count; ++i) {
        new_components[i] = ui_state.components[i];
    }

    // Free old memory and update pointers
    memory_deallocate(ui_state.components,
                     sizeof(UI_Component) * ui_state.component_capacity,
                     Memory_Tag::UI);
    ui_state.components = new_components;
    ui_state.component_capacity = new_capacity;

    CORE_DEBUG("Expanded component registry to %u slots", new_capacity);
    return true;
}

b8 ui_register_component(const UI_Component* component) {
    RUNTIME_ASSERT_MSG(component, "Component cannot be null");
    RUNTIME_ASSERT_MSG(component->name, "Component name cannot be null");
    RUNTIME_ASSERT_MSG(component->on_render, "Component render callback cannot be null");

    if (!ui_state.is_initialized) {
        CORE_ERROR("UI subsystem not initialized");
        return false;
    }

    // Check if component already exists
    if (find_component_by_name(component->name)) {
        CORE_WARN("Component '%s' already registered", component->name);
        return false;
    }

    // Expand registry if needed
    if (ui_state.component_count >= ui_state.component_capacity) {
        if (!expand_component_registry()) {
            return false;
        }
    }

    // Add the component
    ui_state.components[ui_state.component_count] = *component;

    // Call attach callback if provided
    UI_Component* new_component = &ui_state.components[ui_state.component_count];
    if (new_component->on_attach) {
        new_component->on_attach(new_component->user_data);
    }

    ui_state.component_count++;

    CORE_DEBUG("Registered UI component: %s", component->name);
    return true;
}

b8 ui_unregister_component(const char* name) {
    RUNTIME_ASSERT_MSG(name, "Component name cannot be null");

    if (!ui_state.is_initialized) {
        CORE_ERROR("UI subsystem not initialized");
        return false;
    }

    // Find the component
    for (u32 i = 0; i < ui_state.component_count; ++i) {
        if (ui_state.components[i].name && strcmp(ui_state.components[i].name, name) == 0) {
            UI_Component* component = &ui_state.components[i];

            // Call detach callback if provided
            if (component->on_detach) {
                component->on_detach(component->user_data);
            }

            // Remove by shifting remaining components down
            for (u32 j = i; j < ui_state.component_count - 1; ++j) {
                ui_state.components[j] = ui_state.components[j + 1];
            }

            ui_state.component_count--;
            CORE_DEBUG("Unregistered UI component: %s", name);
            return true;
        }
    }

    CORE_WARN("Component '%s' not found for unregistration", name);
    return false;
}

b8 ui_set_component_active(const char* name, b8 active) {
    RUNTIME_ASSERT_MSG(name, "Component name cannot be null");

    if (!ui_state.is_initialized) {
        CORE_ERROR("UI subsystem not initialized");
        return false;
    }

    UI_Component* component = find_component_by_name(name);
    if (component) {
        component->is_active = active;
        CORE_DEBUG("Set component '%s' active state to %s", name, active ? "true" : "false");
        return true;
    }

    CORE_WARN("Component '%s' not found", name);
    return false;
}

const UI_Component* ui_get_component(const char* name) {
    RUNTIME_ASSERT_MSG(name, "Component name cannot be null");

    if (!ui_state.is_initialized) {
        CORE_ERROR("UI subsystem not initialized");
        return nullptr;
    }

    return find_component_by_name(name);
}

void ui_set_menu_callback(UI_MenuCallback callback, void* user_data) {
    ui_state.menu_callback = callback;
    ui_state.menu_user_data = user_data;
    CORE_DEBUG("Set menu callback: %p", (void*)callback);
}

void ui_set_dockspace_enabled(b8 enabled) {
    ui_state.show_dockspace = enabled;
    CORE_DEBUG("Dockspace %s", enabled ? "enabled" : "disabled");
}

void ui_set_custom_titlebar_enabled(b8 enabled) {
    ui_state.custom_titlebar_enabled = enabled;
    CORE_DEBUG("Custom titlebar %s", enabled ? "enabled" : "disabled");
}

b8 ui_set_client_config(Client_UI_Config* client_ui_config) {
    RUNTIME_ASSERT_MSG(client_ui_config, "Client UI config cannot be null");

    if (!ui_state.is_initialized) {
        CORE_ERROR("UI system not initialized");
        return false;
    }

    client_config = client_ui_config;

    // Set menu callback from client
    if (client_config->menu_callback) {
        ui_set_menu_callback((UI_MenuCallback)client_config->menu_callback,
                           client_config->menu_user_data);
    }

    // Infrastructure components are already initialized and registered

    // Register client components as UI components
    for (u32 i = 0; i < client_config->component_count; ++i) {
        const Client_UI_Component* client_comp = &client_config->components[i];

        UI_Component ui_comp = {
            .name = client_comp->name,
            .on_render = (UI_RenderCallback)client_comp->on_render,
            .on_attach = (UI_AttachCallback)client_comp->on_attach,
            .on_detach = (UI_DetachCallback)client_comp->on_detach,
            .user_data = client_comp->user_data,
            .is_active = client_comp->is_active
        };

        if (!ui_register_component(&ui_comp)) {
            CORE_WARN("Failed to register client component: %s", client_comp->name);
        }
    }

    CORE_INFO("Client UI configuration applied with %d components", client_config->component_count);
    return true;
}
