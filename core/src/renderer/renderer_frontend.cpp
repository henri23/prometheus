#include "renderer/renderer_frontend.hpp"
#include "renderer/renderer_backend.hpp"

#include "core/logger.hpp"

// Renderer frontend will manage the backend interface state
struct Renderer_System_State {
    Renderer_Backend backend;
};

internal_variable Renderer_System_State state;

b8 renderer_startup(const char* application_name) {

    if (!renderer_backend_initialize(Renderer_Backend_Type::VULKAN,
            &state.backend)) {

        CORE_INFO("Failed to initialize renderer backend");
        return false;
    }

    state.backend.initialize(&state.backend, application_name);

    CORE_DEBUG("Renderer subsystem initialized");
    return true;
}

void renderer_shutdown() {
    state.backend.shutdown(&state.backend);

    renderer_backend_shutdown(&state.backend);

    CORE_DEBUG("Renderer subsystem shutting down...");
}

void renderer_on_resize(u16 width, u16 height) {

    state.backend.resized(&state.backend, width, height);
}

b8 renderer_begin_frame(f32 delta_t) {
    return state.backend.begin_frame(&state.backend, delta_t);
}

b8 renderer_end_frame(f32 delta_t) {
    b8 result = state.backend.end_frame(&state.backend, delta_t);

    state.backend.frame_number++;
    return result;
}

b8 renderer_draw_frame(Render_Packet* packet) {
    if (renderer_begin_frame(packet->delta_time)) {

        b8 result = renderer_end_frame(packet->delta_time);

        if (!result) {
            CORE_ERROR(
                "renderer_end_frame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}
