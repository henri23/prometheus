#pragma once

#include "vulkan_types.hpp"

// UI-specific image for ImGui texture display
struct Vulkan_UI_Image {
    Vulkan_Image base_image;
    VkSampler sampler;
    VkDescriptorSet descriptor_set;
};

// Create image specifically for ImGui texture use
void vulkan_ui_image_create(
    Vulkan_Context* context,
    u32 width,
    u32 height,
    VkFormat format,
    const void* pixel_data,
    u32 pixel_data_size,
    Vulkan_UI_Image* out_ui_image);

// Destroy UI image (handles descriptor set cleanup)
void vulkan_ui_image_destroy(
    Vulkan_Context* context,
    Vulkan_UI_Image* ui_image);