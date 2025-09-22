#pragma once

#include "renderer/renderer_types.hpp"

b8 vulkan_initialize(Renderer_Backend* backend, const char* app_name);

void vulkan_shutdown(Renderer_Backend* backend);

void vulkan_on_resized(Renderer_Backend* backend, u16 width, u16 height);

b8 vulkan_frame_render(Renderer_Backend* backend, f32 delta_t);

b8 vulkan_frame_present(Renderer_Backend* backend, f32 delta_t);
