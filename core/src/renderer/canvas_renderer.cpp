#include "canvas_renderer.hpp"

#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "renderer_backend.hpp"
#include "vulkan_types.hpp"

#include <imgui_impl_vulkan.h>

struct Canvas_Renderer_State {
    VkDevice device;
    VkPhysicalDevice physical_device;
    VkAllocationCallbacks* allocator;
    VkQueue queue;
    u32 queue_family;

    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkFence render_fence;

    VkRenderPass render_pass;
    VkFormat color_format;

    VkImage color_image;
    VkDeviceMemory color_memory;
    VkImageView color_view;
    VkSampler sampler;
    VkFramebuffer framebuffer;

    ImTextureID texture_id;

    Canvas_Size extent;
    Canvas_Size pending_extent;

    VkClearColorValue clear_color;

    b8 has_pending_resize;
    b8 resources_initialized;
    b8 has_rendered;
    b8 is_initialized;
};

internal_variable Canvas_Renderer_State g_Canvas_State = {};

namespace {

u32 find_memory_type(VkPhysicalDevice physical_device,
    u32 type_filter,
    VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_properties);

    for (u32 i = 0; i < mem_properties.memoryTypeCount; ++i) {
        if ((type_filter & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    RUNTIME_ASSERT_MSG(false, "Failed to find suitable Vulkan memory type for canvas");
    return 0;
}

void destroy_render_target(Canvas_Renderer_State& state) {
    if (state.texture_id) {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)state.texture_id);
        state.texture_id = nullptr;
    }

    if (state.framebuffer) {
        vkDestroyFramebuffer(state.device, state.framebuffer, state.allocator);
        state.framebuffer = VK_NULL_HANDLE;
    }

    if (state.sampler) {
        vkDestroySampler(state.device, state.sampler, state.allocator);
        state.sampler = VK_NULL_HANDLE;
    }

    if (state.color_view) {
        vkDestroyImageView(state.device, state.color_view, state.allocator);
        state.color_view = VK_NULL_HANDLE;
    }

    if (state.color_image) {
        vkDestroyImage(state.device, state.color_image, state.allocator);
        state.color_image = VK_NULL_HANDLE;
    }

    if (state.color_memory) {
        vkFreeMemory(state.device, state.color_memory, state.allocator);
        state.color_memory = VK_NULL_HANDLE;
    }

    state.extent = {0, 0};
    state.resources_initialized = false;
    state.has_rendered = false;
}

b8 create_render_target(Canvas_Renderer_State& state, const Canvas_Size& size) {
    VkImageCreateInfo image_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = size.width;
    image_info.extent.height = size.height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = state.color_format;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateImage(state.device, &image_info, state.allocator, &state.color_image));

    VkMemoryRequirements memory_requirements;
    vkGetImageMemoryRequirements(state.device, state.color_image, &memory_requirements);

    VkMemoryAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(state.physical_device,
        memory_requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(state.device, &alloc_info, state.allocator, &state.color_memory));
    VK_CHECK(vkBindImageMemory(state.device, state.color_image, state.color_memory, 0));

    VkImageViewCreateInfo view_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = state.color_image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = state.color_format;
    view_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY};
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(state.device, &view_info, state.allocator, &state.color_view));

    VkSamplerCreateInfo sampler_info = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    VK_CHECK(vkCreateSampler(state.device, &sampler_info, state.allocator, &state.sampler));

    VkFramebufferCreateInfo framebuffer_info = {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebuffer_info.renderPass = state.render_pass;
    framebuffer_info.attachmentCount = 1;
    framebuffer_info.pAttachments = &state.color_view;
    framebuffer_info.width = size.width;
    framebuffer_info.height = size.height;
    framebuffer_info.layers = 1;

    VK_CHECK(vkCreateFramebuffer(state.device,
        &framebuffer_info,
        state.allocator,
        &state.framebuffer));

    state.texture_id = ImGui_ImplVulkan_AddTexture(state.sampler,
        state.color_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    state.extent = size;
    state.resources_initialized = true;
    state.has_rendered = false;

    return true;
}

b8 create_render_pass(Canvas_Renderer_State& state) {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = state.color_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;

    VkSubpassDependency dependencies[2] = {};

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 2;
    render_pass_info.pDependencies = dependencies;

    VK_CHECK(vkCreateRenderPass(state.device,
        &render_pass_info,
        state.allocator,
        &state.render_pass));

    return true;
}

b8 handle_pending_resize(Canvas_Renderer_State& state) {
    if (!state.has_pending_resize)
        return true;

    Canvas_Size request = state.pending_extent;
    state.pending_extent = {0, 0};
    state.has_pending_resize = false;

    destroy_render_target(state);

    if (request.width == 0 || request.height == 0) {
        return true;
    }

    if (!create_render_target(state, request)) {
        CORE_ERROR("Failed to create canvas render target (%u x %u)",
            request.width,
            request.height);
        return false;
    }

    CORE_DEBUG("Canvas render target resized: %ux%u", request.width, request.height);
    return true;
}

} // namespace

b8 canvas_renderer_initialize() {
    Canvas_Renderer_State& state = g_Canvas_State;

    if (state.is_initialized) {
        CORE_WARN("Canvas renderer already initialized");
        return true;
    }

    state.device = renderer_get_device();
    state.physical_device = renderer_get_physical_device();
    state.allocator = renderer_get_allocator();
    state.queue = renderer_get_queue();
    state.queue_family = renderer_get_queue_family_index();

    if (!state.device || !state.queue) {
        CORE_ERROR("Vulkan device or queue not available for canvas renderer");
        return false;
    }

    state.color_format = VK_FORMAT_R8G8B8A8_UNORM;
    state.clear_color = {{0.1f, 0.1f, 0.12f, 1.0f}};
    state.extent = {0, 0};
    state.pending_extent = {0, 0};
    state.texture_id = nullptr;

    VkCommandPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pool_info.queueFamilyIndex = state.queue_family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(state.device,
        &pool_info,
        state.allocator,
        &state.command_pool));

    VkCommandBufferAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    alloc_info.commandPool = state.command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    VK_CHECK(vkAllocateCommandBuffers(state.device,
        &alloc_info,
        &state.command_buffer));

    VkFenceCreateInfo fence_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    VK_CHECK(vkCreateFence(state.device,
        &fence_info,
        state.allocator,
        &state.render_fence));

    if (!create_render_pass(state)) {
        CORE_ERROR("Failed to create canvas render pass");
        return false;
    }

    state.is_initialized = true;

    CORE_INFO("Canvas renderer initialized");
    return true;
}

void canvas_renderer_shutdown() {
    Canvas_Renderer_State& state = g_Canvas_State;

    if (!state.is_initialized)
        return;

    vkDeviceWaitIdle(state.device);

    destroy_render_target(state);

    if (state.render_pass) {
        vkDestroyRenderPass(state.device, state.render_pass, state.allocator);
        state.render_pass = VK_NULL_HANDLE;
    }

    if (state.command_buffer) {
        vkFreeCommandBuffers(state.device,
            state.command_pool,
            1,
            &state.command_buffer);
        state.command_buffer = VK_NULL_HANDLE;
    }

    if (state.render_fence) {
        vkDestroyFence(state.device, state.render_fence, state.allocator);
        state.render_fence = VK_NULL_HANDLE;
    }

    if (state.command_pool) {
        vkDestroyCommandPool(state.device, state.command_pool, state.allocator);
        state.command_pool = VK_NULL_HANDLE;
    }

    state.is_initialized = false;
    state.resources_initialized = false;
    state.has_rendered = false;
    state.texture_id = nullptr;

    CORE_INFO("Canvas renderer shut down");
}

void canvas_renderer_request_resize(u32 width, u32 height) {
    Canvas_Renderer_State& state = g_Canvas_State;

    if (!state.is_initialized)
        return;

    Canvas_Size request = {width, height};

    if (state.resources_initialized &&
        request.width == state.extent.width &&
        request.height == state.extent.height) {
        return;
    }

    if (state.has_pending_resize &&
        request.width == state.pending_extent.width &&
        request.height == state.pending_extent.height) {
        return;
    }

    state.pending_extent = request;
    state.has_pending_resize = true;
}

Canvas_Size canvas_renderer_get_size() {
    return g_Canvas_State.extent;
}

b8 canvas_renderer_has_output() {
    return g_Canvas_State.has_rendered;
}

ImTextureID canvas_renderer_get_texture_id() {
    return g_Canvas_State.has_rendered ? g_Canvas_State.texture_id : nullptr;
}

b8 canvas_renderer_render(f32 delta_time) {
    (void)delta_time;

    Canvas_Renderer_State& state = g_Canvas_State;

    if (!state.is_initialized)
        return false;

    VK_CHECK(vkWaitForFences(state.device,
        1,
        &state.render_fence,
        VK_TRUE,
        UINT64_MAX));

    if (!handle_pending_resize(state)) {
        return false;
    }

    if (!state.resources_initialized || state.extent.width == 0 || state.extent.height == 0) {
        return true;
    }

    VK_CHECK(vkResetFences(state.device, 1, &state.render_fence));
    VK_CHECK(vkResetCommandBuffer(state.command_buffer, 0));

    VkCommandBufferBeginInfo begin_info = {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(state.command_buffer, &begin_info));

    VkClearValue clear_value = {};
    clear_value.color = state.clear_color;

    VkRenderPassBeginInfo render_pass_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    render_pass_info.renderPass = state.render_pass;
    render_pass_info.framebuffer = state.framebuffer;
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = {state.extent.width, state.extent.height};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_value;

    vkCmdBeginRenderPass(state.command_buffer,
        &render_pass_info,
        VK_SUBPASS_CONTENTS_INLINE);

    vkCmdEndRenderPass(state.command_buffer);

    VK_CHECK(vkEndCommandBuffer(state.command_buffer));

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &state.command_buffer;

    VK_CHECK(vkQueueSubmit(state.queue, 1, &submit_info, state.render_fence));

    state.has_rendered = true;

    return true;
}
