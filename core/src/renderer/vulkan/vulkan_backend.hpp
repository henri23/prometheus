#pragma once

#include "renderer/renderer_types.hpp"
#include <vulkan/vulkan.h>

// Forward declaration
struct Vulkan_Context;

b8 vulkan_initialize(Renderer_Backend* backend, const char* app_name);

void vulkan_shutdown(Renderer_Backend* backend);

void vulkan_on_resized(Renderer_Backend* backend, u16 width, u16 height);

b8 vulkan_frame_render(Renderer_Backend* backend, f32 delta_t);

b8 vulkan_frame_present(Renderer_Backend* backend, f32 delta_t);

// Access to Vulkan context for ImGui integration
Vulkan_Context* vulkan_get_context();

// Get CAD render target texture for ImGui display
VkDescriptorSet vulkan_get_cad_texture();

// Resize CAD render target
void vulkan_resize_cad_render_target(u32 width, u32 height);
