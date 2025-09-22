#pragma once

#include "vulkan_types.hpp"

void vulkan_fence_create(
    Vulkan_Context* context,
    b8 create_signaled,
    Vulkan_Fence* out_fence);

void vulkan_fence_destroy(
    Vulkan_Context* context,
    Vulkan_Fence* fence);

b8 vulkan_fence_wait(
    Vulkan_Context* context,
    Vulkan_Fence* fence,
	u64 timeout_ns);

void vulkan_fence_reset(
    Vulkan_Context* context,
    Vulkan_Fence* fence);
