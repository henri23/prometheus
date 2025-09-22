#pragma once

#include "defines.hpp"

#include <imgui.h>

struct Canvas_Size {
    u32 width;
    u32 height;
};

// Initialize canvas renderer subsystem (off-screen framebuffer)
b8 canvas_renderer_initialize();

// Shutdown canvas renderer and release resources
void canvas_renderer_shutdown();

// Request resize for the off-screen canvas (handled next frame)
void canvas_renderer_request_resize(u32 width, u32 height);

// Get current canvas size (0 if no render target is available)
Canvas_Size canvas_renderer_get_size();

// Query if the canvas image contains rendered data
b8 canvas_renderer_has_output();

// Retrieve texture identifier for ImGui::Image (nullptr if unavailable)
ImTextureID canvas_renderer_get_texture_id();

// Render canvas for current frame (records and submits commands)
b8 canvas_renderer_render(f32 delta_time);
