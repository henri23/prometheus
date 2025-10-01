#include "vulkan_viewport.hpp"
#include "core/logger.hpp"
#include "imgui_impl_vulkan.h"
#include "renderer/vulkan/shaders/vulkan_object_shader.hpp"
#include "vulkan_command_buffer.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_image.hpp"
#include "vulkan_renderpass.hpp"
#include <cmath>

// Size tolerance for resize optimization (avoid micro-resizes during window
// dragging)
#define VIEWPORT_RESIZE_TOLERANCE 8

b8 vulkan_viewport_initialize(Vulkan_Context* context) {
    CORE_DEBUG("Initializing viewport rendering system...");

    // Initialize viewport render target with default size
    u32 default_width = 800;
    u32 default_height = 600;

    // Note: Main target resources are created in vulkan_backend.cpp
    // This function is now just for completeness

    // Note: Descriptor set creation is deferred until first use
    // because ImGui Vulkan backend may not be initialized yet

    CORE_INFO("Viewport rendering system initialized successfully");
    return true;
}

void vulkan_viewport_shutdown(Vulkan_Context* context) {
    CORE_DEBUG("Shutting down viewport rendering system...");

    // Destroy main renderer resources
    // Note: Descriptor set cleanup is now handled by UI layer
    // Destroy sampler
    if (context->main_target.sampler != VK_NULL_HANDLE) {
        vkDestroySampler(
            context->device.logical_device,
            context->main_target.sampler,
            context->allocator);
        CORE_DEBUG("Sampler destroyed");
        context->main_target.sampler = VK_NULL_HANDLE;
    }
    vulkan_framebuffer_destroy(context, &context->main_target.framebuffer);
    vulkan_image_destroy(context, &context->main_target.color_attachment);
    vulkan_image_destroy(context, &context->main_target.depth_attachment);

    CORE_DEBUG("Viewport rendering system shut down");
}

void vulkan_viewport_render(Vulkan_Context* context) {
    // Use dedicated main renderer command buffer
	// We instead use the current frame to render in the off-screen renderer
	// becuase the image_index is needed in the ui flow since we have the 
	// swapchain that is presenting the framebuffers, instead for the off-screen
	// renderer, the swapchain doesn't own anything, and instead it is just 
	// rendering to different framebuffers based of the current_frame 
    Vulkan_Command_Buffer* main_command_buffer =
        &context->main_command_buffers[context->current_frame];

    // Begin recording main renderer commands
    vulkan_command_buffer_begin(main_command_buffer, false, false, false);

    // Begin rendering to the main render target
    // The renderpass will automatically transition from SHADER_READ_ONLY_OPTIMAL
    // to COLOR_ATTACHMENT_OPTIMAL at the beginning, and back to SHADER_READ_ONLY_OPTIMAL
    // at the end (configured in renderpass creation)
    vulkan_renderpass_begin(main_command_buffer,
        &context->main_renderpass,
        context->main_target.framebuffer.handle);

    // Set viewport for main render target
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)context->main_target.width;
    viewport.height = (f32)context->main_target.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    CORE_DEBUG("Setting viewport: %ux%u", context->main_target.width, context->main_target.height);
    vkCmdSetViewport(main_command_buffer->handle, 0, 1, &viewport);

    // Set scissor for main render target
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {context->main_target.width, context->main_target.height};
    vkCmdSetScissor(main_command_buffer->handle, 0, 1, &scissor);

    CORE_DEBUG("Drawing triangle in viewport...");
    vulkan_object_shader_use(context, &context->object_shader);

    VkDeviceSize offsets[1] = {0};

    vkCmdBindVertexBuffers(main_command_buffer->handle,
        0,
        1,
        &context->object_vertex_buffer.handle,
        (VkDeviceSize*)offsets);

    vkCmdBindIndexBuffer(main_command_buffer->handle,
        context->object_index_buffer.handle,
        0,
        VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(main_command_buffer->handle, 3, 1, 0, 0, 0);
    CORE_DEBUG("Triangle draw call issued");
    // For now, just clear the render target - we'll add actual rendering later
    // TODO: Add grid rendering, shape rendering, etc.

    // End rendering to the render target
    vulkan_renderpass_end(main_command_buffer, &context->main_renderpass);

    // The renderpass automatically transitions the image to SHADER_READ_ONLY_OPTIMAL
    // No manual transition needed

    // End command buffer recording
    vulkan_command_buffer_end(main_command_buffer);

    // Submit the main renderer command buffer
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &main_command_buffer->handle;

    VK_CHECK(vkQueueSubmit(context->device.graphics_queue,
        1,
        &submit_info,
        VK_NULL_HANDLE // No fence for now
        ));

    vulkan_command_buffer_update_submitted(main_command_buffer);

    // Wait for main rendering to complete before UI rendering
    vkQueueWaitIdle(context->device.graphics_queue);
}

void vulkan_viewport_resize(Vulkan_Context* context, u32 width, u32 height) {

    // Apply size tolerance to avoid constant resizing during window dragging
    u32 current_width = context->main_target.width;
    u32 current_height = context->main_target.height;

    s32 width_diff = abs((s32)width - (s32)current_width);
    s32 height_diff = abs((s32)height - (s32)current_height);

    if (width_diff <= VIEWPORT_RESIZE_TOLERANCE &&
        height_diff <= VIEWPORT_RESIZE_TOLERANCE) {
        return; // Skip resize if change is too small
    }

    CORE_DEBUG("Resizing viewport from %ux%u to %ux%u",
        current_width,
        current_height,
        width,
        height);

    // Destroy existing resources
    if (context->main_target.descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(context->main_target.descriptor_set);
        context->main_target.descriptor_set = VK_NULL_HANDLE;
    }

    // Destroy sampler
    if (context->main_target.sampler != VK_NULL_HANDLE) {
        vkDestroySampler(
            context->device.logical_device,
            context->main_target.sampler,
            context->allocator);
        CORE_DEBUG("Sampler destroyed");
        context->main_target.sampler = VK_NULL_HANDLE;
    }
    vulkan_framebuffer_destroy(context, &context->main_target.framebuffer);
    vulkan_image_destroy(context, &context->main_target.color_attachment);
    vulkan_image_destroy(context, &context->main_target.depth_attachment);

    // Recreate with new size
    vulkan_image_create(context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &context->main_target.color_attachment);

    vulkan_image_create(context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        context->device.depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &context->main_target.depth_attachment);

    VkImageView attachments_views[] = {
        context->main_target.color_attachment.view,
        context->main_target.depth_attachment.view};

    vulkan_framebuffer_create(context,
        &context->main_renderpass,
        width,
        height,
        2,
        attachments_views,
        &context->main_target.framebuffer);

    // Create linear sampler for main render target
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 1.0f;
    sampler_info.maxAnisotropy = 1.0f;

    VK_CHECK(vkCreateSampler(
        context->device.logical_device,
        &sampler_info,
        context->allocator,
        &context->main_target.sampler));

    CORE_DEBUG("Linear sampler created successfully");

    context->main_target.width = width;
    context->main_target.height = height;

    vulkan_image_transition_layout(context,
        context->main_target.color_attachment.handle,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Update descriptor set after resize
    vulkan_viewport_update_descriptor(context);
}

VkDescriptorSet vulkan_viewport_get_texture(Vulkan_Context* context) {
    // Lazily create descriptor set if it doesn't exist
    // This handles the case where ImGui Vulkan backend wasn't ready during
    // initialization
    if (context->main_target.descriptor_set == VK_NULL_HANDLE) {
        vulkan_viewport_update_descriptor(context);
    }
    return context->main_target.descriptor_set;
}

void vulkan_viewport_update_descriptor(Vulkan_Context* context) {
    // Remove existing descriptor set if it exists
    if (context->main_target.descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(context->main_target.descriptor_set);
        context->main_target.descriptor_set = VK_NULL_HANDLE;
    }

    // Create ImGui descriptor set for displaying as texture
    context->main_target.descriptor_set =
        ImGui_ImplVulkan_AddTexture(context->main_target.sampler,
            context->main_target.color_attachment.view,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    if (context->main_target.descriptor_set == VK_NULL_HANDLE) {
        CORE_ERROR("Failed to create ImGui descriptor set for viewport texture!");
    } else {
        CORE_DEBUG("Viewport descriptor set updated: %p",
            (void*)context->main_target.descriptor_set);
    }
}
