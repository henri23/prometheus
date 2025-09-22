#pragma once
#include "vulkan_types.hpp"

// The renderpass tells Vulkan about the framebuffer attachments, the images
// that we created that we are going to be using while rendering i.e. our color
// attachments and our depth attachments
// In complex applications, the screne is built over many passes, where each
// pass renders a specific part of the scene. As single renderpass object
// encapsulates multiple passes or rendering phases over a single set of output
// images. Each pass withing the renderpass is known as a subpass.
// All drawing must be contained inside a renderpass. The graphics pipeline
// needs to know where its rendering to, so the renderpass object must be created
// before the graphics pipeline so the renderpass can tell it about the images.
void vulkan_renderpass_create(
    Vulkan_Context* context,
    Vulkan_Renderpass* out_renderpass,
    // Definition of the area of the image that we want to render to
    f32 x, f32 y, f32 w, f32 h, // (TODO) Change to Vec4
    f32 r, f32 g, f32 b, f32 a, // Clear color
    f32 depth,
    u32 stencil);

void vulkan_renderpass_destroy(
    Vulkan_Context* context,
    Vulkan_Renderpass* renderpass);

void vulkan_renderpass_begin(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Renderpass* renderpass,
    VkFramebuffer frame_buffer);

void vulkan_renderpass_end(
	Vulkan_Command_Buffer* command_buffer,
	Vulkan_Renderpass* renderpass
);
