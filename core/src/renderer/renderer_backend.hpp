#pragma once

#include "defines.hpp"
#include "vulkan_types.hpp"

// Forward declarations
struct ImDrawData;

b8 renderer_initialize();

void renderer_shutdown();

b8 renderer_draw_frame(ImDrawData* draw_data);

// Vulkan backend initialization for ImGui (called by UI subsystem)
b8 renderer_init_imgui_vulkan();


// Wait for all GPU operations to complete (for safe shutdown)
b8 renderer_wait_idle();

// Trigger swapchain recreation (for window resize events)
void renderer_trigger_swapchain_recreation();

// Get Vulkan context for internal renderer use
VkDevice renderer_get_device();
VkPhysicalDevice renderer_get_physical_device();
VkQueue renderer_get_queue();
VkAllocationCallbacks* renderer_get_allocator();
VkCommandPool renderer_get_command_pool();
u32 renderer_get_queue_family_index();
