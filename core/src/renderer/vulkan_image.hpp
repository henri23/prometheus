#pragma once

#include "defines.hpp"
#include "vulkan_types.hpp"

enum class Image_Format {
    NONE = 0,
    RGBA,
    RGBA32F
};

struct Vulkan_Image {
    u32 width;
    u32 height;

    VkImage image;
    VkImageView image_view;
    VkDeviceMemory memory;
    VkSampler sampler;

    Image_Format format;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    u64 aligned_size;

    VkDescriptorSet descriptor_set;

    const char* filepath;
};

// Create image from file path
b8 vulkan_image_create_from_file(
    Vulkan_Image* image,
    const char* path);

// Create image from raw data
b8 vulkan_image_create_from_data(
    Vulkan_Image* image,
    u32 width,
    u32 height,
    Image_Format format,
    const void* data);

// Set new image data
b8 vulkan_image_set_data(
    Vulkan_Image* image,
    const void* data);

// Resize the image
b8 vulkan_image_resize(
    Vulkan_Image* image,
    u32 width,
    u32 height);

// Release all resources
void vulkan_image_destroy(Vulkan_Image* image);

// Decode image data from memory buffer
void* vulkan_image_decode(
    const void* data,
    u64 length,
    u32* out_width,
    u32* out_height);

// Helper functions
INTERNAL_FUNC u32 vulkan_image_get_bytes_per_pixel(Image_Format format);
INTERNAL_FUNC VkFormat vulkan_image_format_to_vk_format(Image_Format format);
INTERNAL_FUNC u32 vulkan_image_get_memory_type(
    VkMemoryPropertyFlags properties,
    u32 type_bits);