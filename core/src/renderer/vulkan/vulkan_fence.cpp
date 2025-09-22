#include "vulkan_fence.hpp"
#include "core/logger.hpp"

// We need a fence wrapper, because by just debuggin the fence itself, we will
// see only a vulkan pointer to the fence, but we do not have any information
// whether it has been signaled or not. Moreover if we do not implement some 
// logic that if the fence has been signaled to return instead of waiting inde
// finetly, we might run in a situation where we are waiting until we reach a 
// timeout. This could be easily prevented by keeping track of the state of the
// fence.
void vulkan_fence_create(
    Vulkan_Context* context,
    b8 create_signaled,
    Vulkan_Fence* out_fence) {

    out_fence->is_signaled = create_signaled;

    VkFenceCreateInfo fence_create_info = {VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    if (out_fence->is_signaled) {
        fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    }

    VK_ENSURE_SUCCESS(
        vkCreateFence(
            context->device.logical_device,
            &fence_create_info,
            context->allocator,
            &out_fence->handle));
}

void vulkan_fence_destroy(
    Vulkan_Context* context,
    Vulkan_Fence* fence) {

    if (fence->handle) {
        vkDestroyFence(
            context->device.logical_device,
            fence->handle,
            context->allocator);

        fence->handle = nullptr;
    }

    fence->is_signaled = false;
}

b8 vulkan_fence_wait(
    Vulkan_Context* context,
    Vulkan_Fence* fence,
    u64 timeout_ns) {

    if (!fence->is_signaled) {
        VkResult result = vkWaitForFences(
            context->device.logical_device,
            1,
            &fence->handle,
            true,
            timeout_ns);

        switch (result) {

        case VK_SUCCESS:
            fence->is_signaled = true;
            return true;
        case VK_TIMEOUT:
            CORE_WARN("vk_fence_wait - Timed out");
            break;
        case VK_ERROR_DEVICE_LOST:
            CORE_ERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            CORE_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            CORE_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY");
            break;
        default:
            CORE_ERROR("vk_fence_wait - An unknown error has occured");
            break;
        }
    } else {
        return true;
    }

    return false;
}

void vulkan_fence_reset(
    Vulkan_Context* context,
    Vulkan_Fence* fence) {

    if (fence->is_signaled) {
        VK_ENSURE_SUCCESS(
            vkResetFences(
                context->device.logical_device,
                1,
                &fence->handle));

        fence->is_signaled = false;
    }
}
