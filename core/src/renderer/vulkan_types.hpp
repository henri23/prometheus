#pragma once

#include "defines.hpp"
#include <vulkan/vulkan.h>
#include "core/asserts.hpp"
#include <imgui_impl_vulkan.h>

#define VK_CHECK(expr) RUNTIME_ASSERT(expr == VK_SUCCESS);

struct Vulkan_Context {
#ifdef DEBUG_BUILD
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
	VkAllocationCallbacks* allocator;
	VkInstance instance;
	VkPhysicalDevice physical_device;
	VkDevice logical_device;
	u32 queue_family;
	VkSurfaceKHR surface;
	VkQueue queue;
	VkPipelineCache pipeline_cache;
	VkDescriptorPool descriptor_pool;
	ImGui_ImplVulkanH_Window main_window_data;
	b8 swapchain_rebuild;
};

#define VK_DEVICE_LEVEL_FUNCTION(device, name)                        \
    PFN_##name name = (PFN_##name)vkGetDeviceProcAddr(device, #name); \
    RUNTIME_ASSERT_MSG(name, "Could not load device-level Vulkan function");

#define VK_INSTANCE_LEVEL_FUNCTION(instance, name)                        \
    PFN_##name name = (PFN_##name)vkGetInstanceProcAddr(instance, #name); \
    RUNTIME_ASSERT_MSG(name, "Could not load instance-level Vulkan function");\

