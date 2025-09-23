#include "vulkan_renderpass.hpp"
#include "core/logger.hpp"
#include "memory/memory.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

void vulkan_renderpass_create(
    Vulkan_Context* context,
    Vulkan_Renderpass* out_renderpass,
    // Definition of the area of the image that we want to render to
    f32 x, f32 y, f32 w, f32 h, // (TODO) Change to Vec4
    f32 r, f32 g, f32 b, f32 a, // Clear color
    f32 depth,
    u32 stencil) {

    out_renderpass->x = x;
    out_renderpass->y = y;
    out_renderpass->w = w;
    out_renderpass->h = h;

    out_renderpass->r = r;
    out_renderpass->g = g;
    out_renderpass->b = b;
    out_renderpass->a = a;

    out_renderpass->depth = depth;
    out_renderpass->stencil = stencil;

    // Main subpass - Graphics pipeline. For the moment the renderpass will
    // have only 1 pass
    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    // Prepare two attachments for color and depth
    constexpr u32 attachment_description_count = 2;
    VkAttachmentDescription attachment_descriptions[attachment_description_count];

    // We need to define what attachments will be used during rendering, i.e.
    // images like color/depth buffers
    VkAttachmentDescription color_attachment;

    // Format for color iamges already specified during swapchain creation
    color_attachment.format = context->swapchain.image_format.format;

    // Set each pixel to be samples only once by the shaders
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // When the renderpass starts, clear the color attachment (typically to
    // black of specified color)
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    // When the renderpass stops, store the results. This settings is very
    // important since we want to store the results of the renderpass to allow
    // the swapchain to present them
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Since we do not use stencil operations, we do not care about these two
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // After render pass, the image will be ready for presentation
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    color_attachment.flags = 0;

    attachment_descriptions[0] = color_attachment;

    // Specify that during the subpass. attachment with index 0 will be used
    // with color-optimzed layout
    VkAttachmentReference color_attachment_reference;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Bind the color attachment to the subpass
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    VkAttachmentDescription depth_attachment = {};
    depth_attachment.format = context->device.depth_format;

    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;

    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // We do not vare about the z-buffer contents after rendering
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // We do not do stencil operations
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    attachment_descriptions[1] = depth_attachment;

    VkAttachmentReference depth_attachment_reference;
    depth_attachment_reference.attachment = 1;
    depth_attachment_reference.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_reference;

    // TODO: Specify other attachment types like input, resolve, preserve
    subpass.pDepthStencilAttachment = &depth_attachment_reference;

    subpass.inputAttachmentCount = 0;
    subpass.pInputAttachments = nullptr;

    subpass.pResolveAttachments = nullptr;

    subpass.preserveAttachmentCount = 0;
    subpass.pPreserveAttachments = nullptr;

    // TODO: I do not understand this part good enough yet
    VkSubpassDependency dependency;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependency.dependencyFlags = 0;

    VkRenderPassCreateInfo create_info =
        {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};

    // Specify the attachments
    create_info.attachmentCount = attachment_description_count;
    create_info.pAttachments = attachment_descriptions;

    // Define the subpasses. In this case only 1 subpass
    create_info.subpassCount = 1;
    create_info.pSubpasses = &subpass;

    // Dependecies
    create_info.dependencyCount = 1;
    create_info.pDependencies = &dependency;

    create_info.pNext = nullptr;
    create_info.flags = 0;

    VK_ENSURE_SUCCESS(
        vkCreateRenderPass(
            context->device.logical_device,
            &create_info,
            context->allocator,
            &out_renderpass->handle));

    CORE_INFO("Renderpass object created successfully");
}

void vulkan_renderpass_destroy(
    Vulkan_Context* context,
    Vulkan_Renderpass* renderpass) {

    if (renderpass && renderpass->handle) {
        vkDestroyRenderPass(
            context->device.logical_device,
            renderpass->handle,
            context->allocator);
        renderpass->handle = nullptr;
    }
}

void vulkan_renderpass_begin(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Renderpass* renderpass,
    VkFramebuffer frame_buffer) {

    VkRenderPassBeginInfo begin_info =
        {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};

    begin_info.renderPass = renderpass->handle;
    begin_info.framebuffer = frame_buffer;

    begin_info.renderArea.offset.x = renderpass->x;
    begin_info.renderArea.offset.y = renderpass->y;
    begin_info.renderArea.extent.width = renderpass->w;
    begin_info.renderArea.extent.height = renderpass->h;

    VkClearValue clear_values[2];
    memory_zero(clear_values, sizeof(VkClearValue) * 2);
    clear_values[0].color.float32[0] = renderpass->r;
    clear_values[0].color.float32[1] = renderpass->g;
    clear_values[0].color.float32[2] = renderpass->b;
    clear_values[0].color.float32[3] = renderpass->a;

    clear_values[1].depthStencil.depth = renderpass->depth;
    clear_values[1].depthStencil.stencil = renderpass->stencil;

    begin_info.clearValueCount = 2;
    begin_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(
        command_buffer->handle,
        &begin_info,
        VK_SUBPASS_CONTENTS_INLINE);

    command_buffer->state = Command_Buffer_State::IN_RENDER_PASS;
}

void vulkan_renderpass_end(
    Vulkan_Command_Buffer* command_buffer,
    Vulkan_Renderpass* renderpass) {

    vkCmdEndRenderPass(command_buffer->handle);
    command_buffer->state = Command_Buffer_State::RECORDING;
}
