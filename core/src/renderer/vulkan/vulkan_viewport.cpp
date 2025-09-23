#include "vulkan_viewport.hpp"
#include "vulkan_render_target.hpp"
#include "vulkan_command_buffer.hpp"
#include "core/logger.hpp"
#include "imgui_impl_vulkan.h"
#include <cmath>

// Size tolerance for resize optimization (avoid micro-resizes during window dragging)
#define VIEWPORT_RESIZE_TOLERANCE 8

b8 vulkan_viewport_initialize(Vulkan_Context* context) {
    CORE_DEBUG("Initializing viewport rendering system...");

    // Initialize viewport render target with default size
    u32 default_width = 800;
    u32 default_height = 600;

    vulkan_render_target_create(
        context,
        default_width,
        default_height,
        VK_FORMAT_R8G8B8A8_UNORM,  // Color format
        context->device.depth_format,
        &context->cad_render_target);

    // Note: Descriptor set creation is deferred until first use
    // because ImGui Vulkan backend may not be initialized yet

    CORE_INFO("Viewport rendering system initialized successfully");
    return true;
}

void vulkan_viewport_shutdown(Vulkan_Context* context) {
    CORE_DEBUG("Shutting down viewport rendering system...");

    // Destroy the viewport render target
    vulkan_render_target_destroy(context, &context->cad_render_target);

    CORE_DEBUG("Viewport rendering system shut down");
}

void vulkan_viewport_render(Vulkan_Context* context) {
    // Use dedicated CAD command buffer instead of the main one
    Vulkan_Command_Buffer* cad_command_buffer = &context->cad_command_buffers[context->current_frame];

    // Begin recording CAD commands
    vulkan_command_buffer_begin(cad_command_buffer, false, false, false);

    // Begin rendering to the CAD render target
    vulkan_render_target_begin(
        cad_command_buffer,
        &context->cad_render_target,
        0.15f, 0.15f, 0.15f, 1.0f // Dark gray background
    );

    // For now, just clear the render target - we'll add actual rendering later
    // TODO: Add grid rendering, shape rendering, etc.

    // End rendering to the render target
    vulkan_render_target_end(cad_command_buffer, &context->cad_render_target);

    // End command buffer recording
    vulkan_command_buffer_end(cad_command_buffer);

    // Submit the CAD command buffer
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cad_command_buffer->handle;

    VK_ENSURE_SUCCESS(vkQueueSubmit(
        context->device.graphics_queue,
        1,
        &submit_info,
        VK_NULL_HANDLE // No fence for now
    ));

    // Wait for CAD rendering to complete before UI rendering
    vkQueueWaitIdle(context->device.graphics_queue);
}

void vulkan_viewport_resize(
    Vulkan_Context* context,
    u32 width,
    u32 height) {

    // Apply size tolerance to avoid constant resizing during window dragging
    u32 current_width = context->cad_render_target.width;
    u32 current_height = context->cad_render_target.height;

    s32 width_diff = abs((s32)width - (s32)current_width);
    s32 height_diff = abs((s32)height - (s32)current_height);

    if (width_diff <= VIEWPORT_RESIZE_TOLERANCE && height_diff <= VIEWPORT_RESIZE_TOLERANCE) {
        return; // Skip resize if change is too small
    }

    CORE_DEBUG("Resizing viewport from %ux%u to %ux%u",
              current_width, current_height, width, height);

    // Resize the render target
    vulkan_render_target_resize(context, &context->cad_render_target, width, height);

    // Update descriptor set after resize
    vulkan_viewport_update_descriptor(context);
}

VkDescriptorSet vulkan_viewport_get_texture(Vulkan_Context* context) {
    // Lazily create descriptor set if it doesn't exist
    // This handles the case where ImGui Vulkan backend wasn't ready during initialization
    if (context->cad_render_target.descriptor_set == VK_NULL_HANDLE) {
        vulkan_viewport_update_descriptor(context);
    }
    return context->cad_render_target.descriptor_set;
}

void vulkan_viewport_update_descriptor(Vulkan_Context* context) {
    // Update the descriptor set for ImGui display
    vulkan_render_target_update_descriptor(context, &context->cad_render_target);

    CORE_DEBUG("Viewport descriptor set updated: %p",
              (void*)context->cad_render_target.descriptor_set);
}