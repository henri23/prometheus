#include "vulkan_render_target.hpp"
#include "vulkan_image.hpp"
#include "core/logger.hpp"
#include "imgui_impl_vulkan.h"

void vulkan_render_target_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    VkFormat color_format,
    VkFormat depth_format,
    Vulkan_Render_Target* out_render_target) {

    CORE_DEBUG("Creating render target: %ux%u", width, height);

    // Initialize all fields to safe defaults
    out_render_target->width = width;
    out_render_target->height = height;
    out_render_target->color_format = color_format;
    out_render_target->depth_format = depth_format;
    out_render_target->framebuffer = VK_NULL_HANDLE;
    out_render_target->renderpass = VK_NULL_HANDLE;
    out_render_target->sampler = VK_NULL_HANDLE;
    out_render_target->descriptor_set = VK_NULL_HANDLE;

    // Create color attachment
    vulkan_image_create(
        context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        color_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &out_render_target->color_attachment);

    // Create depth attachment
    vulkan_image_create(
        context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        depth_format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &out_render_target->depth_attachment);

    // Create renderpass
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = color_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = depth_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription attachments[] = {color_attachment, depth_attachment};

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 2;
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VK_ENSURE_SUCCESS(vkCreateRenderPass(
        context->device.logical_device,
        &render_pass_info,
        context->allocator,
        &out_render_target->renderpass));

    // Create framebuffer
    VkImageView attachments_views[] = {
        out_render_target->color_attachment.view,
        out_render_target->depth_attachment.view
    };

    VkFramebufferCreateInfo framebuffer_info = {};
    framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_info.renderPass = out_render_target->renderpass;
    framebuffer_info.attachmentCount = 2;
    framebuffer_info.pAttachments = attachments_views;
    framebuffer_info.width = width;
    framebuffer_info.height = height;
    framebuffer_info.layers = 1;

    VK_ENSURE_SUCCESS(vkCreateFramebuffer(
        context->device.logical_device,
        &framebuffer_info,
        context->allocator,
        &out_render_target->framebuffer));

    // Create sampler for texture sampling
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

    VK_ENSURE_SUCCESS(vkCreateSampler(
        context->device.logical_device,
        &sampler_info,
        context->allocator,
        &out_render_target->sampler));

    // Transition color attachment to shader read-only layout
    // This is necessary because when the render target is created, the image starts in
    // VK_IMAGE_LAYOUT_UNDEFINED but the descriptor set expects VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    vulkan_image_transition_layout(
        context,
        out_render_target->color_attachment.handle,
        color_format,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    // Initialize descriptor set as null - will be created later when needed
    out_render_target->descriptor_set = VK_NULL_HANDLE;

    CORE_DEBUG("Render target created successfully");
}

void vulkan_render_target_destroy(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target) {

    CORE_DEBUG("Destroying render target...");

    // Remove ImGui descriptor set
    if (render_target->descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(render_target->descriptor_set);
        render_target->descriptor_set = VK_NULL_HANDLE;
    }

    // Destroy sampler (this one is owned by the render target)
    if (render_target->sampler != VK_NULL_HANDLE) {
        vkDestroySampler(
            context->device.logical_device,
            render_target->sampler,
            context->allocator);
        render_target->sampler = VK_NULL_HANDLE;
    }

    // Destroy framebuffer
    if (render_target->framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(
            context->device.logical_device,
            render_target->framebuffer,
            context->allocator);
        render_target->framebuffer = VK_NULL_HANDLE;
    }

    // Destroy renderpass
    if (render_target->renderpass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(
            context->device.logical_device,
            render_target->renderpass,
            context->allocator);
        render_target->renderpass = VK_NULL_HANDLE;
    }

    // Destroy attachments
    vulkan_image_destroy(context, &render_target->color_attachment);
    vulkan_image_destroy(context, &render_target->depth_attachment);

    CORE_DEBUG("Render target destroyed");
}

void vulkan_render_target_resize(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target,
    u32 new_width,
    u32 new_height) {

    if (new_width == render_target->width && new_height == render_target->height) {
        return; // No change needed
    }

    CORE_DEBUG("Resizing render target from %ux%u to %ux%u",
              render_target->width, render_target->height,
              new_width, new_height);

    // Store formats
    VkFormat color_format = render_target->color_format;
    VkFormat depth_format = render_target->depth_format;

    // Destroy current render target
    vulkan_render_target_destroy(context, render_target);

    // Recreate with new size
    vulkan_render_target_create(context, new_width, new_height, color_format, depth_format, render_target);
}

void vulkan_render_target_begin(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Render_Target* render_target,
    f32 clear_r, f32 clear_g, f32 clear_b, f32 clear_a) {

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = render_target->renderpass;
    render_pass_info.framebuffer = render_target->framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = {render_target->width, render_target->height};

    VkClearValue clear_values[2];
    clear_values[0].color = {{clear_r, clear_g, clear_b, clear_a}};
    clear_values[1].depthStencil = {1.0f, 0};

    render_pass_info.clearValueCount = 2;
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(command_buffer->handle, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // Set viewport
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (f32)render_target->width;
    viewport.height = (f32)render_target->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);

    // Set scissor
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {render_target->width, render_target->height};
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);
}

void vulkan_render_target_end(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Render_Target* render_target) {

    vkCmdEndRenderPass(command_buffer->handle);
}

void vulkan_render_target_update_descriptor(
    Vulkan_Context* context,
    Vulkan_Render_Target* render_target) {

    // Remove existing descriptor set if it exists
    if (render_target->descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(render_target->descriptor_set);
        render_target->descriptor_set = VK_NULL_HANDLE;
    }

    // Create or update ImGui descriptor set for displaying as texture
    // Note: This assumes the image layout is already SHADER_READ_ONLY_OPTIMAL
    // The layout transition happens in vulkan_render_target_end()
    render_target->descriptor_set = ImGui_ImplVulkan_AddTexture(
        render_target->sampler,
        render_target->color_attachment.view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    CORE_DEBUG("Render target descriptor set updated: %p", (void*)render_target->descriptor_set);
}