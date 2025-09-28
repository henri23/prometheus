#pragma once

#include "vulkan_types.hpp"

/**
 * Vulkan Sampler Module
 * Provides reusable sampler creation functions for different use cases
 */

/**
 * Create a linear sampler suitable for texture display
 * Commonly used for displaying render targets as textures in UI
 * @param context - Vulkan context
 * @param out_sampler - Output sampler handle
 */
void vulkan_sampler_create_linear(
    Vulkan_Context* context,
    VkSampler* out_sampler);

/**
 * Create a nearest neighbor sampler
 * Suitable for pixel-perfect rendering or debug visualization
 * @param context - Vulkan context
 * @param out_sampler - Output sampler handle
 */
void vulkan_sampler_create_nearest(
    Vulkan_Context* context,
    VkSampler* out_sampler);

/**
 * Destroy a sampler
 * @param context - Vulkan context
 * @param sampler - Sampler to destroy
 */
void vulkan_sampler_destroy(
    Vulkan_Context* context,
    VkSampler sampler);