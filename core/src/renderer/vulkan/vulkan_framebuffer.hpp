#pragma once

#include "vulkan_types.hpp"

// FIY: Vulkan refers to resources attached to the swapchain as attachments
void vulkan_framebuffer_create(
    Vulkan_Context* context,
    Vulkan_Renderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachment_count,
    VkImageView* attachments,
    Vulkan_Framebuffer* out_framebuffer);

void vulkan_framebuffer_destroy(
    Vulkan_Context* context,
    Vulkan_Framebuffer* framebuffer);
