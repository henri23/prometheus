#pragma once

#include "defines.hpp"
#include "vulkan_types.hpp"

/**
 * Vulkan Viewport Module
 * Manages off-screen rendering for CAD viewports
 * Provides clean separation between viewport rendering and swapchain presentation
 */

/**
 * Initialize the viewport rendering system
 * Creates render targets and command buffers for off-screen rendering
 * @param context - Vulkan context
 * @return true if successful, false otherwise
 */
PROMETHEUS_API b8 vulkan_viewport_initialize(Vulkan_Context* context);

/**
 * Shutdown the viewport rendering system
 * Cleans up all viewport-related Vulkan resources
 * @param context - Vulkan context
 */
PROMETHEUS_API void vulkan_viewport_shutdown(Vulkan_Context* context);

/**
 * Render viewport content to off-screen render target
 * Handles all CAD viewport rendering (grid, shapes, etc.)
 * @param context - Vulkan context
 */
PROMETHEUS_API void vulkan_viewport_render(Vulkan_Context* context);

/**
 * Resize the viewport render target
 * Includes size tolerance to avoid constant resizing during window dragging
 * @param context - Vulkan context
 * @param width - New viewport width
 * @param height - New viewport height
 */
PROMETHEUS_API void vulkan_viewport_resize(
    Vulkan_Context* context,
    u32 width,
    u32 height);

/**
 * Get the viewport texture for ImGui display
 * Returns the descriptor set for displaying viewport content in UI
 * @param context - Vulkan context
 * @return VkDescriptorSet for ImGui texture display
 */
PROMETHEUS_API VkDescriptorSet vulkan_viewport_get_texture(Vulkan_Context* context);

/**
 * Update viewport descriptor set for ImGui display
 * Called after render target changes to refresh texture display
 * @param context - Vulkan context
 */
PROMETHEUS_API void vulkan_viewport_update_descriptor(Vulkan_Context* context);