#include "vulkan_framebuffer.hpp"
#include "core/logger.hpp"
#include "memory/memory.hpp"

// The framebuffer in Vulkan is a container that ties attachments (image views)
// used during a render pass. It defines where the actual rendering output goes
// for a particular frame
// It binds image views to a render pass instance. it acts as the target for
// rendering commands inside a render pass and it defines the dimensions of the
// rendering region.
void vulkan_framebuffer_create(
    Vulkan_Context* context,
    Vulkan_Renderpass* renderpass,
    u32 width,
    u32 height,
    u32 attachment_count,
    VkImageView* attachments,
    Vulkan_Framebuffer* out_framebuffer) {

    out_framebuffer->attachments = static_cast<VkImageView*>(
        memory_allocate(
            sizeof(VkImageView) * attachment_count,
            Memory_Tag::RENDERER));

	for(u32 i = 0; i < attachment_count; ++i)
		out_framebuffer->attachments[i] = attachments[i];

    out_framebuffer->renderpass = renderpass;
    out_framebuffer->attachment_count = attachment_count;

    VkFramebufferCreateInfo framebuffer_info =
        {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};

    framebuffer_info.renderPass = renderpass->handle;
    framebuffer_info.attachmentCount = attachment_count;
    framebuffer_info.pAttachments = out_framebuffer->attachments;
    framebuffer_info.width = width;
    framebuffer_info.height = height;
    framebuffer_info.layers = 1;

    VK_ENSURE_SUCCESS(
        vkCreateFramebuffer(
            context->device.logical_device,
            &framebuffer_info,
            context->allocator,
            &out_framebuffer->handle));

    // ENGINE_DEBUG("Vulkan framebuffer created with size { %u, %u }",
    //              width,
    //              height);
}

void vulkan_framebuffer_destroy(
    Vulkan_Context* context,
    Vulkan_Framebuffer* framebuffer) {

    vkDestroyFramebuffer(
        context->device.logical_device,
        framebuffer->handle,
        context->allocator);

    if (framebuffer->attachments) {
        memory_deallocate(
            framebuffer->attachments,
            sizeof(VkImageView) * framebuffer->attachment_count,
            Memory_Tag::RENDERER);
    }

    framebuffer->handle = nullptr;
    framebuffer->attachment_count = 0;
    framebuffer->renderpass = nullptr;
}
