#include "vulkan_image.hpp"
#include "renderer/vulkan/vulkan_types.hpp"
#include "renderer/vulkan/vulkan_command_buffer.hpp"

#include "core/logger.hpp"
#include "imgui_impl_vulkan.h"

// Images are memory blocks allocated in the device that include information
// about the content too. Contrary to buffers which are just chunks of data
void vulkan_image_create(
    Vulkan_Context* context,
    VkImageType image_type,
    u32 width,
    u32 height,
    VkFormat format, // the format of the image
    VkImageTiling tiling,
    VkImageUsageFlags usage, // how will the image be used
    VkMemoryPropertyFlags memory_flags,
    b32 create_view,
    VkImageAspectFlags view_aspect_flags,
    Vulkan_Image* out_image) {

    // Vulkan does not automatically allocate memory for images. Instead we:
    // 1. Create an image with vkCreateImage
    // 2. Query its memory requirements with vkGetImageMemoryRequirements
    // 3. Allocate device memory manually using vkAllocateMemory
    // 4. Bind the memory to the image with vkBindImageMemory

    out_image->width = width;
    out_image->height = height;

    VkImageCreateInfo image_create_info =
        {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};

    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.width = width;
    image_create_info.extent.height = height;

    // For 2D images, depth = 1, for 3D images depth > 1, i.e. 64. If we are
    // creating a 3D texture (like volumetric smoke) this tells Vulkan how deep
    // the texture is, kinda like stacking 2D slices of the texture along the
    // Z-Axis
    image_create_info.extent.depth = 1; // TODO: Make config.

    // Mipmaps are smaller version of an image, used for texture filtering at
    // different distances.
    // Level 0: 1024x1024
    // Level 1: 512x512
    // Level 2: 256x256
    // 			...
    // Level N: 1x1
    // If level is set to 1, no mipmapping will be used.
    image_create_info.mipLevels = 1; // Single mip level for textures

    // Used if we want an array of 2D images, not a 3D volume. Obviously without
    // specifying this, Vulkan cannot know how to interpret the other dimension.
    // Used in texture arrays for animation frames, cube maps, layered
    // framebuffers etc. In this case we will not have an array of textures, but
    // single textures by setting arrayLayers = 1
    image_create_info.arrayLayers = 1; // TODO: Make config.

    image_create_info.format = format;

    // Options for tiling:
    // - VK_IMAGE_TILING_OPTIMAL: Image is stored in a way optimized for GPU.
    // 	 The CPU cannot read/write directly as it does not know the opt. layout
    // 	 Bets for rendering textures, framebuffers, depth buffers
    // - VK_IMAGE_TILING_LINEAR: Image is laid out in memory line-by-line. The
    // 	 CPU can access directly, but is slower that the GPU one.
    image_create_info.tiling = tiling;

    // LAYOUT_UNDEFINED means that this image will not be transitioned to
    // anther memory layout
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // Usage is a bitmask that describes what the image is meant to do:
    // - VK_IMAGE_USAGE_SAMPLED_BIT: We want to read from the image in a shader
    // - VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT: We want to draw into this image
    // - VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT: Used for depth or stencil
    //   buffer (z-buffer)
    // - VK_IMAGE_USAGE_TRANSFER_SRC_BIT: We will copy from this image to another
    // - VK_IMAGE_USAGE_TRANSFER_DST_BIT: We will copy to this image i.e. upload
    //	 texture from CPU
    image_create_info.usage = usage;

    // MSSA (multisample anti-alliasing) is a technique used to smooth jagged
    // edges by sampling multiple times per pixel and averaging the result. The
    // VK_SAMPLE_COUNT_X_BIT X value represents the number of times a pixel will
    // be shaded (and averaged over those samples)
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT; // TODO: Make config.

    // sharingMode describes whether this image will be accesses by more than
    // one queue
    image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_ENSURE_SUCCESS(
        vkCreateImage(
            context->device.logical_device,
            &image_create_info,
            context->allocator,
            &out_image->handle));

    // Query the memory requirements for the created image.
    VkMemoryRequirements memory_requirements;

    // vkGetImageMemoryRequirements tells us how much GPU memory an images needs.
    // It also says how this image should be aligned and which memory types are
    // valid. This information is given before we bing the memory to the image.
    vkGetImageMemoryRequirements(
        context->device.logical_device,
        out_image->handle,
        &memory_requirements);

    // Since we get the memory types from the query above, we must query the
    // device in order to get the device's memory index for that specific type
    s32 memory_type = context->find_memory_index(
        memory_requirements.memoryTypeBits,
        memory_flags);

    if (memory_type == -1) {
        CORE_ERROR("Required memory type not found. Image is not valid");
    }

    // Allocate the memory
    VkMemoryAllocateInfo memory_allocate_info =
        {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
    memory_allocate_info.allocationSize = memory_requirements.size;
    memory_allocate_info.memoryTypeIndex = memory_type;

    VK_ENSURE_SUCCESS(
        vkAllocateMemory(
            context->device.logical_device,
            &memory_allocate_info,
            context->allocator,
            &out_image->memory));

    // Bind the memory to the image
    VK_ENSURE_SUCCESS(
        vkBindImageMemory(
            context->device.logical_device,
            out_image->handle,
            out_image->memory,
            0) // TODO: Config. memory offeset, i.e. image pools
    );

    if (create_view) {
        out_image->view = nullptr;
        vulkan_image_view_create(
            context,
            format,
            out_image,
            view_aspect_flags);
    }
}

void vulkan_image_view_create(
    Vulkan_Context* context,
    VkFormat format,
    Vulkan_Image* image,
    VkImageAspectFlags aspect_flags) {

    VkImageViewCreateInfo view_create_info =
        {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_create_info.format = format;
    view_create_info.image = image->handle;
    view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D; // TODO: Make config.
    view_create_info.subresourceRange.aspectMask = aspect_flags;

    // TODO: Make config.
    view_create_info.subresourceRange.baseMipLevel = 0;
    view_create_info.subresourceRange.levelCount = 1;
    view_create_info.subresourceRange.baseArrayLayer = 0;
    view_create_info.subresourceRange.layerCount = 1;

    VK_ENSURE_SUCCESS(
        vkCreateImageView(
            context->device.logical_device,
            &view_create_info,
            context->allocator,
            &image->view));
}

void vulkan_image_destroy(
    Vulkan_Context* context,
    Vulkan_Image* image) {

    // Remove this log if the logger gets too crowded
    CORE_DEBUG("Destroying vulkan image...");

    // Clean up ImGui descriptor set if it exists
    if (image->descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(image->descriptor_set);
        image->descriptor_set = VK_NULL_HANDLE;
    }

    // Note: Don't destroy sampler here if it's shared (like imgui_linear_sampler)
    // Only destroy samplers that were created specifically for this image
    // The shared samplers will be destroyed by their owners
    image->sampler = VK_NULL_HANDLE;

    if (image->view) {
        vkDestroyImageView(
            context->device.logical_device,
            image->view,
			context->allocator);

		image->view = nullptr;
    }

    if (image->memory) {
        vkFreeMemory(
            context->device.logical_device,
            image->memory,
			context->allocator);

		image->memory = nullptr;
    }

    if (image->handle) {
        vkDestroyImage(
            context->device.logical_device,
            image->handle,
			context->allocator);

		image->handle = nullptr;
    }

    CORE_DEBUG("Vulkan image destroyed");
}

void vulkan_image_create_for_imgui(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    VkFormat format,
    const void* pixel_data,
    u32 pixel_data_size,
    Vulkan_Image* out_image) {

    // Create the Vulkan image
    vulkan_image_create(
        context,
        VK_IMAGE_TYPE_2D,
        width,
        height,
        format,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        true,
        VK_IMAGE_ASPECT_COLOR_BIT,
        out_image);

    // Initialize descriptor set to NULL handle
    out_image->descriptor_set = VK_NULL_HANDLE;
    out_image->sampler = VK_NULL_HANDLE;

    // Upload pixel data if provided
    if (pixel_data && pixel_data_size > 0) {
        // Create staging buffer
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;

        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = pixel_data_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VK_ENSURE_SUCCESS(vkCreateBuffer(
            context->device.logical_device,
            &buffer_info,
            context->allocator,
            &staging_buffer));

        VkMemoryRequirements mem_requirements;
        vkGetBufferMemoryRequirements(
            context->device.logical_device,
            staging_buffer,
            &mem_requirements);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = mem_requirements.size;
        s32 memory_type_index = context->find_memory_index(
            mem_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (memory_type_index == -1) {
            CORE_ERROR("Failed to find suitable memory type for staging buffer");
            vkDestroyBuffer(context->device.logical_device, staging_buffer, context->allocator);
            return;
        }
        alloc_info.memoryTypeIndex = memory_type_index;

        VK_ENSURE_SUCCESS(vkAllocateMemory(
            context->device.logical_device,
            &alloc_info,
            context->allocator,
            &staging_buffer_memory));

        VK_ENSURE_SUCCESS(vkBindBufferMemory(
            context->device.logical_device,
            staging_buffer,
            staging_buffer_memory,
            0));

        // Copy pixel data to staging buffer
        void* data;
        vkMapMemory(
            context->device.logical_device,
            staging_buffer_memory,
            0,
            pixel_data_size,
            0,
            &data);
        memcpy(data, pixel_data, pixel_data_size);
        vkUnmapMemory(context->device.logical_device, staging_buffer_memory);

        // Transition image layout for transfer destination
        vulkan_image_transition_layout(
            context,
            out_image->handle,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // Copy buffer to image
        Vulkan_Command_Buffer command_buffer;
        vulkan_command_buffer_startup_single_use(
            context,
            context->device.graphics_command_pool,
            &command_buffer);

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {
            out_image->width,
            out_image->height,
            1
        };

        vkCmdCopyBufferToImage(
            command_buffer.handle,
            staging_buffer,
            out_image->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);

        vulkan_command_buffer_end_single_use(
            context,
            context->device.graphics_command_pool,
            &command_buffer,
            context->device.graphics_queue);

        // Transition image layout for shader reading
        vulkan_image_transition_layout(
            context,
            out_image->handle,
            VK_FORMAT_R8G8B8A8_UNORM,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // Clean up staging buffer
        vkDestroyBuffer(context->device.logical_device, staging_buffer, context->allocator);
        vkFreeMemory(context->device.logical_device, staging_buffer_memory, context->allocator);
    }

    // Note: We're using the shared imgui_linear_sampler, but we don't own it
    // so we don't set it in the image structure to avoid cleanup issues
    out_image->sampler = VK_NULL_HANDLE;

    // Create descriptor set using ImGui's function
    out_image->descriptor_set = ImGui_ImplVulkan_AddTexture(
        context->imgui_linear_sampler,
        out_image->view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    CORE_DEBUG("ImGui image created: %ux%u", width, height);
}

void vulkan_image_destroy_imgui(Vulkan_Image* image) {
    if (image->descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(image->descriptor_set);
        image->descriptor_set = VK_NULL_HANDLE;
    }

    // Note: The Vulkan image resources (view, memory, handle) still need to be
    // cleaned up by the regular vulkan_image_destroy function
}

void vulkan_image_transition_layout(
    Vulkan_Context* context,
    VkImage image,
    VkFormat format,
    VkImageLayout old_layout,
    VkImageLayout new_layout) {

    Vulkan_Command_Buffer command_buffer;
    vulkan_command_buffer_startup_single_use(
        context,
        context->device.graphics_command_pool,
        &command_buffer);

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;

    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
        new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED &&
               new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        // Direct transition from undefined to shader read-only for render targets
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        CORE_ERROR("Unsupported layout transition!");
        return;
    }

    vkCmdPipelineBarrier(
        command_buffer.handle,
        source_stage, destination_stage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vulkan_command_buffer_end_single_use(
        context,
        context->device.graphics_command_pool,
        &command_buffer,
        context->device.graphics_queue);
}
