#include "vulkan_sampler.hpp"
#include "core/logger.hpp"

void vulkan_sampler_create_linear(
    Vulkan_Context* context,
    VkSampler* out_sampler) {

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
        out_sampler));

    CORE_DEBUG("Linear sampler created successfully");
}

void vulkan_sampler_create_nearest(
    Vulkan_Context* context,
    VkSampler* out_sampler) {

    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
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
        out_sampler));

    CORE_DEBUG("Nearest sampler created successfully");
}

void vulkan_sampler_destroy(
    Vulkan_Context* context,
    VkSampler sampler) {

    if (sampler != VK_NULL_HANDLE) {
        vkDestroySampler(
            context->device.logical_device,
            sampler,
            context->allocator);
        CORE_DEBUG("Sampler destroyed");
    }
}