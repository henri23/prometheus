#pragma once

#include "vulkan_types.hpp"

// Create an off-screen render target for CAD viewport
void vulkan_render_target_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    VkFormat color_format,
    VkFormat depth_format,
    Vulkan_Render_Target* out_render_target);

// Destroy render target and clean up resources
void vulkan_render_target_destroy(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target);

// Resize render target (recreates framebuffer and attachments)
void vulkan_render_target_resize(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target,
    u32 new_width,
    u32 new_height);

// Begin rendering to the render target
void vulkan_render_target_begin(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Render_Target* render_target,
    f32 clear_r, f32 clear_g, f32 clear_b, f32 clear_a);

// End rendering to the render target
void vulkan_render_target_end(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Render_Target* render_target);

// Update ImGui descriptor set for displaying render target as texture
void vulkan_render_target_update_descriptor(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target);