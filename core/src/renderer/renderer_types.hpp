#pragma once

#include "defines.hpp"
// This file contains declarations that are going to exposed to multiple
// subsystems Initially it contained the renderer backend defition too

// Renderer_backend is the interface of the renderer classes
struct Renderer_Backend {
    u64 frame_number;

    b8 (*initialize)(Renderer_Backend* backend, const char* app_name);

    void (*shutdown)(Renderer_Backend* backend);

    void (*resized)(Renderer_Backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(Renderer_Backend* backend, f32 delta_t);

    b8 (*end_frame)(Renderer_Backend* backend, f32 delta_t);

    // UI Image Management
    b8 (*create_ui_image)(Renderer_Backend* backend, u32 width, u32 height, const void* pixel_data, u32 pixel_data_size, struct UI_Image_Resource* out_image_resource);
    void (*destroy_ui_image)(Renderer_Backend* backend, struct UI_Image_Resource* resource);
};

// Render packets may contain info needed to render a frame
// i.e. list of meshes, camera information etc.
struct Render_Packet {
    f32 delta_time;
};

enum class Renderer_Backend_Type { VULKAN, OPENGL, DIRECTX };
