#pragma once

#include "core/asserts.hpp"
#include "defines.hpp"

#include "containers/auto_array.hpp"

#include <vulkan/vulkan.h>

#define VK_ENSURE_SUCCESS(expr) RUNTIME_ASSERT(expr == VK_SUCCESS);

struct Vulkan_Swapchain_Support_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    u32 formats_count;
    VkSurfaceFormatKHR* formats;
    u32 present_modes_count;
    VkPresentModeKHR* present_modes;
};

struct Vulkan_Device {
    VkPhysicalDevice physical_device; // Handle ptr to physical device
    VkDevice logical_device;          // Handle to be destroyed

    // Family queue indices
    u32 graphics_queue_index;
    u32 transfer_queue_index;
    u32 compute_queue_index;
    u32 present_queue_index;

    // Physical device informations
    VkPhysicalDeviceProperties physical_device_properties;
    VkPhysicalDeviceFeatures physical_device_features;
    VkPhysicalDeviceMemoryProperties physical_device_memory;

    Vulkan_Swapchain_Support_Info swapchain_info;

    VkFormat depth_format;

    // Queue handles
    VkQueue presentation_queue;
    VkQueue graphics_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;
};

struct Vulkan_Image {
    VkImage handle;
    VkImageView view;
    VkDeviceMemory memory; // Handle to the memory allocated by the image
    u32 width;
    u32 height;

    // For sampling in shaders/ImGui
    VkSampler sampler;
    VkDescriptorSet descriptor_set;
};

// Off-screen render target with ImGui display capability
struct Vulkan_Render_Target {
    u32 width;
    u32 height;
    VkFormat color_format;
    VkFormat depth_format;

    // Color attachment
    Vulkan_Image color_attachment;

    // Depth attachment
    Vulkan_Image depth_attachment;

    // Framebuffer and renderpass
    VkFramebuffer framebuffer;
    VkRenderPass renderpass;

    // For displaying in ImGui
    VkSampler sampler;
    VkDescriptorSet descriptor_set;
};

// Finite state machine of the renderpass
enum class Renderpass_State {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDIN_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

struct Vulkan_Renderpass {
    VkRenderPass handle;

    f32 x, y, w, h;
    f32 r, g, b, a;

    f32 depth;
    u32 stencil;

    Renderpass_State state;
};

struct Vulkan_Framebuffer {
    VkFramebuffer handle;
    u32 attachment_count;
    VkImageView* attachments;
    Vulkan_Renderpass* renderpass;
};

struct Vulkan_Swapchain {
    VkSwapchainKHR handle;
    u32 max_in_flight_frames;

    u32 image_count;
    VkImage* images;    // array of VkImages. Automatically created and cleaned
    VkImageView* views; // array of Views, struct that lets us access the images

    Auto_Array<Vulkan_Framebuffer> framebuffers;

    VkSurfaceFormatKHR image_format;
    VkExtent2D extent;

    Vulkan_Image depth_attachment;
};

enum class Command_Buffer_State {
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
};

struct Vulkan_Command_Buffer {
    VkCommandBuffer handle;

    Command_Buffer_State state;
};

struct Vulkan_Fence {
    VkFence handle;
    b8 is_signaled;
};

struct Vulkan_Shader_Stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
};

struct Vulkan_Pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
};

constexpr u32 OBJECT_SHADER_STAGE_COUNT = 2;

struct Vulkan_Object_Shader {
    // The shader stage count is for vertex and fragment shaders
    Vulkan_Shader_Stage stages[OBJECT_SHADER_STAGE_COUNT];

    Vulkan_Pipeline pipeline;
};

struct Vulkan_Context {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkAllocationCallbacks* allocator;
    VkPhysicalDevice
        physical_device; // Implicitly destroyed destroying VkInstance

    u32 framebuffer_width;
    u32 framebuffer_height;

    // Keep two replicas for last time and current. If these two are out of
    // sync, a resize event has ocurred
    u64 framebuffer_size_generation;
    u64 framebuffer_size_last_generation;

#ifdef DEBUG_BUILD
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

    u32 image_count;
    u32 image_index;
    u64 current_frame;

    b8 recreating_swapchain;

    Vulkan_Object_Shader object_shader;

    Vulkan_Swapchain swapchain;
    Vulkan_Device device;
    Vulkan_Renderpass main_renderpass;

    // CAD viewport off-screen rendering
    Vulkan_Render_Target cad_render_target;

    Auto_Array<Vulkan_Command_Buffer> graphics_command_buffers;

    // CAD-specific command buffers for off-screen rendering
    Auto_Array<Vulkan_Command_Buffer> cad_command_buffers;

    Auto_Array<VkSemaphore> image_available_semaphores;
    Auto_Array<VkSemaphore> render_finished_semaphores;

    u32 in_flight_fence_count;

    Auto_Array<Vulkan_Fence> in_flight_fences;
    // Keep information about the fences of the images currently in flight. The
    // fences are not owned by this array
    Auto_Array<Vulkan_Fence*> images_in_flight;

    // ImGui integration components
    VkDescriptorPool imgui_descriptor_pool;
    VkDescriptorSetLayout imgui_descriptor_set_layout;
    VkSampler imgui_linear_sampler;

    s32 (*find_memory_index)(u32 type_filter, u32 property_flags);
};

struct Vulkan_Physical_Device_Requirements {
    b8 graphics;
    b8 present;
    b8 compute;
    b8 transfer;
    b8 discrete_gpu;
    b8 sampler_anisotropy;
    Auto_Array<const char*>* device_extension_names;
};

#define VK_DEVICE_LEVEL_FUNCTION(device, name)                                 \
    PFN_##name name = (PFN_##name)vkGetDeviceProcAddr(device, #name);          \
    RUNTIME_ASSERT_MSG(name, "Could not load device-level Vulkan function");

#define VK_INSTANCE_LEVEL_FUNCTION(instance, name)                             \
    PFN_##name name = (PFN_##name)vkGetInstanceProcAddr(instance, #name);      \
    RUNTIME_ASSERT_MSG(name, "Could not load instance-level Vulkan function");
