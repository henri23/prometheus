#include "vulkan_image.hpp"

#include "core/logger.hpp"
#include "core/asserts.hpp"
#include "memory/memory.hpp"
#include "renderer_backend.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

INTERNAL_FUNC u32 vulkan_image_get_memory_type(
    VkMemoryPropertyFlags properties,
    u32 type_bits) {

    VkPhysicalDeviceMemoryProperties prop;
    vkGetPhysicalDeviceMemoryProperties(renderer_get_physical_device(), &prop);

    for (u32 i = 0; i < prop.memoryTypeCount; i++) {
        if ((prop.memoryTypes[i].propertyFlags & properties) == properties &&
            type_bits & (1 << i)) {
            return i;
        }
    }

    return 0xFFFFFFFF;
}

INTERNAL_FUNC u32 vulkan_image_get_bytes_per_pixel(Image_Format format) {
    switch (format) {
        case Image_Format::RGBA:    return 4;
        case Image_Format::RGBA32F: return 16;
        case Image_Format::NONE:    return 0;
    }
    return 0;
}

INTERNAL_FUNC VkFormat vulkan_image_format_to_vk_format(Image_Format format) {
    switch (format) {
        case Image_Format::RGBA:    return VK_FORMAT_R8G8B8A8_UNORM;
        case Image_Format::RGBA32F: return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Image_Format::NONE:    return VK_FORMAT_UNDEFINED;
    }
    return VK_FORMAT_UNDEFINED;
}

INTERNAL_FUNC b8 vulkan_image_allocate_memory(
    Vulkan_Image* image,
    u64 size) {

    VkResult err;
    VkFormat vulkan_format = vulkan_image_format_to_vk_format(image->format);

    // Create the Image
    {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = vulkan_format;
        info.extent.width = image->width;
        info.extent.height = image->height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        err = vkCreateImage(
            renderer_get_device(),
            &info,
            renderer_get_allocator(),
            &image->image);
        VK_CHECK(err);

        VkMemoryRequirements req;
        vkGetImageMemoryRequirements(renderer_get_device(), image->image, &req);

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = vulkan_image_get_memory_type(
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            req.memoryTypeBits);

        err = vkAllocateMemory(
            renderer_get_device(),
            &alloc_info,
            renderer_get_allocator(),
            &image->memory);
        VK_CHECK(err);

        err = vkBindImageMemory(
            renderer_get_device(),
            image->image,
            image->memory,
            0);
        VK_CHECK(err);
    }

    // Create the Image View
    {
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.image = image->image;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = vulkan_format;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;

        err = vkCreateImageView(
            renderer_get_device(),
            &info,
            renderer_get_allocator(),
            &image->image_view);
        VK_CHECK(err);
    }

    // Create sampler
    {
        VkSamplerCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        info.magFilter = VK_FILTER_LINEAR;
        info.minFilter = VK_FILTER_LINEAR;
        info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        info.minLod = -1000;
        info.maxLod = 1000;
        info.maxAnisotropy = 1.0f;

        err = vkCreateSampler(
            renderer_get_device(),
            &info,
            renderer_get_allocator(),
            &image->sampler);
        VK_CHECK(err);
    }

    // Create the Descriptor Set using ImGui
    image->descriptor_set = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(
        image->sampler,
        image->image_view,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return true;
}

b8 vulkan_image_create_from_file(
    Vulkan_Image* image,
    const char* path) {

    RUNTIME_ASSERT_MSG(image, "vulkan_image_create_from_file requires valid image pointer");
    RUNTIME_ASSERT_MSG(path, "vulkan_image_create_from_file requires valid path");

    // Zero initialize the struct
    *image = {};
    image->filepath = path;

    s32 width, height, channels;
    u8* data = nullptr;

    if (stbi_is_hdr(path)) {
        data = (u8*)stbi_loadf(path, &width, &height, &channels, 4);
        image->format = Image_Format::RGBA32F;
    } else {
        data = stbi_load(path, &width, &height, &channels, 4);
        image->format = Image_Format::RGBA;
    }

    if (!data) {
        CORE_ERROR("Failed to load image: %s", path);
        return false;
    }

    image->width = (u32)width;
    image->height = (u32)height;

    u64 size = image->width * image->height * vulkan_image_get_bytes_per_pixel(image->format);

    if (!vulkan_image_allocate_memory(image, size)) {
        stbi_image_free(data);
        return false;
    }

    if (!vulkan_image_set_data(image, data)) {
        stbi_image_free(data);
        vulkan_image_destroy(image);
        return false;
    }

    stbi_image_free(data);
    return true;
}

b8 vulkan_image_create_from_data(
    Vulkan_Image* image,
    u32 width,
    u32 height,
    Image_Format format,
    const void* data) {

    RUNTIME_ASSERT_MSG(image, "vulkan_image_create_from_data requires valid image pointer");

    // Zero initialize the struct
    *image = {};
    image->width = width;
    image->height = height;
    image->format = format;
    image->filepath = nullptr;

    u64 size = width * height * vulkan_image_get_bytes_per_pixel(format);

    if (!vulkan_image_allocate_memory(image, size)) {
        return false;
    }

    if (data) {
        if (!vulkan_image_set_data(image, data)) {
            vulkan_image_destroy(image);
            return false;
        }
    }

    return true;
}

b8 vulkan_image_set_data(
    Vulkan_Image* image,
    const void* data) {

    RUNTIME_ASSERT_MSG(image, "vulkan_image_set_data requires valid image pointer");
    RUNTIME_ASSERT_MSG(data, "vulkan_image_set_data requires valid data pointer");

    u64 upload_size = image->width * image->height * vulkan_image_get_bytes_per_pixel(image->format);
    VkResult err;

    if (!image->staging_buffer) {
        // Create the Upload Buffer
        VkBufferCreateInfo buffer_info = {};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = upload_size;
        buffer_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        err = vkCreateBuffer(
            renderer_get_device(),
            &buffer_info,
            renderer_get_allocator(),
            &image->staging_buffer);
        VK_CHECK(err);

        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(renderer_get_device(), image->staging_buffer, &req);
        image->aligned_size = req.size;

        VkMemoryAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        alloc_info.allocationSize = req.size;
        alloc_info.memoryTypeIndex = vulkan_image_get_memory_type(
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            req.memoryTypeBits);

        err = vkAllocateMemory(
            renderer_get_device(),
            &alloc_info,
            renderer_get_allocator(),
            &image->staging_buffer_memory);
        VK_CHECK(err);

        err = vkBindBufferMemory(
            renderer_get_device(),
            image->staging_buffer,
            image->staging_buffer_memory,
            0);
        VK_CHECK(err);
    }

    // Upload to Buffer
    {
        char* map = nullptr;
        err = vkMapMemory(
            renderer_get_device(),
            image->staging_buffer_memory,
            0,
            image->aligned_size,
            0,
            (void**)(&map));
        VK_CHECK(err);

        // Use custom memory copy instead of memcpy
        u8* src = (u8*)data;
        u8* dst = (u8*)map;
        for (u64 i = 0; i < upload_size; ++i) {
            dst[i] = src[i];
        }

        VkMappedMemoryRange range = {};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = image->staging_buffer_memory;
        range.size = image->aligned_size;

        err = vkFlushMappedMemoryRanges(renderer_get_device(), 1, &range);
        VK_CHECK(err);
        vkUnmapMemory(renderer_get_device(), image->staging_buffer_memory);
    }

    // Copy to Image
    {
        // For simplicity, we'll submit commands immediately
        // In a production system, you might want to batch these
        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandPool = renderer_get_command_pool();
        alloc_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        err = vkAllocateCommandBuffers(renderer_get_device(), &alloc_info, &command_buffer);
        VK_CHECK(err);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        err = vkBeginCommandBuffer(command_buffer, &begin_info);
        VK_CHECK(err);

        VkImageMemoryBarrier copy_barrier = {};
        copy_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        copy_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        copy_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        copy_barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        copy_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        copy_barrier.image = image->image;
        copy_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copy_barrier.subresourceRange.levelCount = 1;
        copy_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_HOST_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &copy_barrier);

        VkBufferImageCopy region = {};
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.layerCount = 1;
        region.imageExtent.width = image->width;
        region.imageExtent.height = image->height;
        region.imageExtent.depth = 1;

        vkCmdCopyBufferToImage(
            command_buffer,
            image->staging_buffer,
            image->image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region);

        VkImageMemoryBarrier use_barrier = {};
        use_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        use_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        use_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        use_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        use_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        use_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        use_barrier.image = image->image;
        use_barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        use_barrier.subresourceRange.levelCount = 1;
        use_barrier.subresourceRange.layerCount = 1;

        vkCmdPipelineBarrier(
            command_buffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &use_barrier);

        err = vkEndCommandBuffer(command_buffer);
        VK_CHECK(err);

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;

        err = vkQueueSubmit(renderer_get_queue(), 1, &submit_info, VK_NULL_HANDLE);
        VK_CHECK(err);

        err = vkQueueWaitIdle(renderer_get_queue());
        VK_CHECK(err);

        vkFreeCommandBuffers(renderer_get_device(), renderer_get_command_pool(), 1, &command_buffer);
    }

    return true;
}

b8 vulkan_image_resize(
    Vulkan_Image* image,
    u32 width,
    u32 height) {

    RUNTIME_ASSERT_MSG(image, "vulkan_image_resize requires valid image pointer");

    if (image->image && image->width == width && image->height == height) {
        return true;
    }

    image->width = width;
    image->height = height;

    // Wait for device to be idle before destroying resources
    renderer_wait_idle();

    // Destroy existing resources
    if (image->sampler) {
        vkDestroySampler(renderer_get_device(), image->sampler, renderer_get_allocator());
        image->sampler = VK_NULL_HANDLE;
    }
    if (image->image_view) {
        vkDestroyImageView(renderer_get_device(), image->image_view, renderer_get_allocator());
        image->image_view = VK_NULL_HANDLE;
    }
    if (image->image) {
        vkDestroyImage(renderer_get_device(), image->image, renderer_get_allocator());
        image->image = VK_NULL_HANDLE;
    }
    if (image->memory) {
        vkFreeMemory(renderer_get_device(), image->memory, renderer_get_allocator());
        image->memory = VK_NULL_HANDLE;
    }

    u64 size = width * height * vulkan_image_get_bytes_per_pixel(image->format);
    return vulkan_image_allocate_memory(image, size);
}

void vulkan_image_destroy(Vulkan_Image* image) {
    if (!image) {
        return;
    }

    // Wait for device to be idle before destroying resources
    renderer_wait_idle();

    if (image->sampler) {
        vkDestroySampler(renderer_get_device(), image->sampler, renderer_get_allocator());
    }
    if (image->image_view) {
        vkDestroyImageView(renderer_get_device(), image->image_view, renderer_get_allocator());
    }
    if (image->image) {
        vkDestroyImage(renderer_get_device(), image->image, renderer_get_allocator());
    }
    if (image->memory) {
        vkFreeMemory(renderer_get_device(), image->memory, renderer_get_allocator());
    }
    if (image->staging_buffer) {
        vkDestroyBuffer(renderer_get_device(), image->staging_buffer, renderer_get_allocator());
    }
    if (image->staging_buffer_memory) {
        vkFreeMemory(renderer_get_device(), image->staging_buffer_memory, renderer_get_allocator());
    }

    // Zero out the struct
    *image = {};
}

void* vulkan_image_decode(
    const void* data,
    u64 length,
    u32* out_width,
    u32* out_height) {

    RUNTIME_ASSERT_MSG(data, "vulkan_image_decode requires valid data pointer");
    RUNTIME_ASSERT_MSG(out_width, "vulkan_image_decode requires valid out_width pointer");
    RUNTIME_ASSERT_MSG(out_height, "vulkan_image_decode requires valid out_height pointer");

    s32 width, height, channels;
    u8* result = stbi_load_from_memory(
        (const stbi_uc*)data,
        (s32)length,
        &width,
        &height,
        &channels,
        4);

    if (!result) {
        CORE_ERROR("Failed to decode image data");
        return nullptr;
    }

    *out_width = (u32)width;
    *out_height = (u32)height;

    return result;
}