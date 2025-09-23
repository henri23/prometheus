#include "core/logger.hpp"

#include "renderer/renderer_types.hpp"
#include "vulkan_command_buffer.hpp"
#include "vulkan_device.hpp"
#include "vulkan_fence.hpp"
#include "vulkan_framebuffer.hpp"
#include "vulkan_platform.hpp"
#include "vulkan_renderpass.hpp"
#include "vulkan_swapchain.hpp"
#include "vulkan_types.hpp"
#include "vulkan_utils.hpp"

#include "containers/auto_array.hpp"
#include "core/application.hpp"
#include "ui/ui.hpp"
#include "ui/ui_titlebar.hpp"
#include "imgui_impl_vulkan.h"

#include "shaders/vulkan_object_shader.hpp"
#include "vulkan_viewport.hpp"
#include <cstring>

internal_variable Vulkan_Context context;
internal_variable u32 cached_framebuffer_width = 0;
internal_variable u32 cached_framebuffer_height = 0;

// Forward declare messenger callback
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

// Debug mode helper private functions
INTERNAL_FUNC b8 vulkan_create_debug_logger(VkInstance* instance);
INTERNAL_FUNC b8 vulkan_enable_validation_layers(
    Auto_Array<const char*>* required_layers_array);

// Needed for z-buffer image format selection in vulkan_device
INTERNAL_FUNC s32 find_memory_index(u32 type_filter, u32 property_flags);

// Graphics presentation operations
INTERNAL_FUNC void create_command_buffers(Renderer_Backend* backend);
INTERNAL_FUNC void create_cad_command_buffers(Vulkan_Context* context);
INTERNAL_FUNC void create_framebuffers(Renderer_Backend* backend,
    Vulkan_Swapchain* swapchain,
    Vulkan_Renderpass* renderpass);
INTERNAL_FUNC b8 present_frame(Renderer_Backend* backend);
INTERNAL_FUNC b8 get_next_image_index(Renderer_Backend* backend);

// ImGui integration functions
INTERNAL_FUNC b8 create_ui_library_resources(Vulkan_Context* context);
INTERNAL_FUNC void destroy_ui_library_resources(Vulkan_Context* context);

// The recreate_swapchain function is called both when a window resize event
// has ocurred and was published by the platform layer, or when a graphics ops.
// (i.e. present or get_next_image_index) finished with a non-optimal result
// code, which require the swapchain recreation. The flag is_resized_event
// descriminates between these two cases and makes sure not to overwrite
// renderpass size or read cached values, which are != 0 only when resize events
// occur.
INTERNAL_FUNC b8 recreate_swapchain(Renderer_Backend* backend, b8 is_resized_event);

b8 vulkan_initialize(Renderer_Backend* backend, const char* app_name) {

    // Function pointer assignment
    context.find_memory_index = find_memory_index;

    // TODO: Custom allocator with memory arenas (Ryan Fleury tutorial)
    context.allocator = nullptr;

    // TODO: I do not like that the renderer calls the application layer,
    // since the dependency should be inverse.
    application_get_framebuffer_size(&cached_framebuffer_width,
        &cached_framebuffer_height);

    context.framebuffer_width =
        (cached_framebuffer_width != 0) ? cached_framebuffer_width : 1280;

    context.framebuffer_height =
        (cached_framebuffer_height != 0) ? cached_framebuffer_height : 720;

    cached_framebuffer_width = 0;
    cached_framebuffer_height = 0;

    VkApplicationInfo app_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO};

    app_info.pNext = nullptr;
    app_info.pApplicationName = app_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Koala engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    // The API version should be set to the absolute minimum version of Vulkan
    // that the game engine requires to run, not to the version of the header
    // that is being used for development. This allows a wide assortment of
    // devices and platforms to run the koala engine
    app_info.apiVersion = VK_API_VERSION_1_2;

    // CreateInfo struct tells the Vulkan driver which global extensions
    // and validation layers to use.
    VkInstanceCreateInfo createInfo = {VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

    createInfo.pApplicationInfo = &app_info;

    Auto_Array<const char*> required_extensions_array;

    // Get platform specific extensions (includes VK_KHR_surface and others)
    platform_get_required_extensions(&required_extensions_array);

    Auto_Array<const char*> required_layers_array;

// Only enable validation layer in debug builds
#ifdef DEBUG_BUILD
    // Add debug extensions
    required_extensions_array.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    CORE_DEBUG("Required VULKAN extensions:");
    for (u32 i = 0; i < required_extensions_array.length; ++i) {
        CORE_DEBUG(required_extensions_array[i]);
    }

    // Add validation layers
    vulkan_enable_validation_layers(&required_layers_array);
#endif

    // In Vulkan, applications need to explicitly specify the extensions that
    // they are going to use, and so the driver disables the extensions that
    // will not be used, so that the application cannot accidently start using
    // an extension in runtime
    createInfo.enabledExtensionCount = required_extensions_array.length;
    createInfo.ppEnabledExtensionNames = required_extensions_array.data;

    createInfo.enabledLayerCount = required_layers_array.length;
    createInfo.ppEnabledLayerNames = required_layers_array.data;

    VK_ENSURE_SUCCESS(
        vkCreateInstance(&createInfo, context.allocator, &context.instance));

#ifdef DEBUG_BUILD
    // Depends on the instance
    vulkan_create_debug_logger(&context.instance);
#endif

    Auto_Array<const char*> device_level_extension_requirements;

    // The swapchain is a device specific property (whether it supports it
    // or it doesn't) so we need to query specificly for the swapchain support
    // the device that we chose to use
    device_level_extension_requirements.push_back(
        VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    // Setup Vulkan device
    Vulkan_Physical_Device_Requirements device_requirements;
    device_requirements.compute = true;
    device_requirements.sampler_anisotropy = true;
    device_requirements.graphics = true;
    device_requirements.transfer = true;
    device_requirements.present = true;
    device_requirements.discrete_gpu = true;
    device_requirements.device_extension_names =
        &device_level_extension_requirements;

    // Create platform specific surface. Since the surface creation will
    // depend on the platform API, it is best that it is implemented in
    // the platform layer
    if (!platform_create_vulkan_surface(&context)) {
        CORE_FATAL("Failed to create platform specific surface");

        return false;
    }

    // Select physical device and create logical device
    if (!vulkan_device_initialize(&context, &device_requirements)) {
        CORE_FATAL(
            "No device that fulfills all the requirements was found in the "
            "machine");
        return false;
    }

    vulkan_swapchain_create(&context,
        context.framebuffer_width,
        context.framebuffer_height,
        &context.swapchain);

    vulkan_renderpass_create(&context,
        &context.main_renderpass,
        0,
        0,
        context.framebuffer_width,
        context.framebuffer_height,
        0.0f,
        0.0f,
        0.3f,
        1.0f,
        1.0f,
        0);

    // Allocate the framebuffers
    context.swapchain.framebuffers.resize(context.swapchain.image_count);

    create_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    // Create CAD command buffers
    create_cad_command_buffers(&context);

    context.image_available_semaphores.resize(
        context.swapchain.max_in_flight_frames);

    context.render_finished_semaphores.resize(context.swapchain.image_count);

    context.in_flight_fences.resize(context.swapchain.max_in_flight_frames);

    VkSemaphoreCreateInfo semaphore_create_info = {
        VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};

    for (u8 i = 0; i < context.swapchain.max_in_flight_frames; ++i) {
        vkCreateSemaphore(context.device.logical_device,
            &semaphore_create_info,
            context.allocator,
            &context.image_available_semaphores[i]);

        // Create the fence in a signaled state, indicating that the first
        // frame has been "rendered". This will prevent the application from
        // waiting indefinetelly, because during boot-up there isn't any frame
        // to render. However we set this state to true, to trigger the next
        // frame rendering.
        vulkan_fence_create(&context, true, &context.in_flight_fences[i]);
    }

    context.images_in_flight.resize(context.swapchain.image_count);
    // At this point in time, the images_in_flight fences are not yet created,
    // so we clear the array first. Basically the value should be nullptr when
    // not used
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vkCreateSemaphore(context.device.logical_device,
            &semaphore_create_info,
            context.allocator,
            &context.render_finished_semaphores[i]);

        context.images_in_flight[i] = nullptr;
    }

    // Create builtin shaders
    // TODO: Temporarily disabled shader loading
    // if (!vulkan_object_shader_create(&context, &context.object_shader)) {
    //     CORE_ERROR("Error loading built-in object shader");
    //     return false;
    // }

    // Create ImGui components
    if (!create_ui_library_resources(&context)) {
        CORE_ERROR("Failed to create ImGui Vulkan components");
        return false;
    }

    // Initialize viewport rendering system
    if (!vulkan_viewport_initialize(&context)) {
        CORE_ERROR("Failed to initialize viewport rendering system");
        return false;
    }

    CORE_INFO("Vulkan backend initialized");

    return true;
}

void vulkan_shutdown(Renderer_Backend* backend) {
    // NOTE:	We might get problems when trying to shutdown the renderer while
    //			there are graphic operation still going on. First, it is
    //better 			to wait until all operations have completed, so we do not get
    //			errors.
    vkDeviceWaitIdle(context.device.logical_device);

    // Wait for any pending CAD operations to complete
    vkQueueWaitIdle(context.device.graphics_queue);

    // Reset command pools IMMEDIATELY to invalidate all command buffers
    // This must be done before any resource destruction to avoid
    // "resource still in use by command buffer" errors
    vkResetCommandPool(context.device.logical_device, context.device.graphics_command_pool, 0);

    // Clean up viewport descriptor set BEFORE ImGui shutdown
    if (context.cad_render_target.descriptor_set != VK_NULL_HANDLE) {
        ImGui_ImplVulkan_RemoveTexture(context.cad_render_target.descriptor_set);
        context.cad_render_target.descriptor_set = VK_NULL_HANDLE;
        CORE_DEBUG("Viewport descriptor set cleaned up early");
    }

    // Clean up UI image descriptor sets before ImGui shutdown
    // This must be done here to prevent crashes during UI shutdown
    CORE_DEBUG("Cleaning up UI Vulkan resources before ImGui shutdown...");
    ui_titlebar_cleanup_vulkan_resources();

    // Shutdown ImGui Vulkan backend BEFORE destroying the device
    // This must happen while the Vulkan device is still valid
    ImGui_ImplVulkan_Shutdown();

    // Destroy ImGui components
    destroy_ui_library_resources(&context);

    // Shutdown viewport rendering system
    vulkan_viewport_shutdown(&context);

    // Destroy shader modules
    // TODO: Temporarily disabled shader cleanup
    // vulkan_object_shader_destroy(&context, &context.object_shader);

    // Destroy sync objects
    for (u8 i = 0; i < context.swapchain.max_in_flight_frames; ++i) {
        vkDestroySemaphore(context.device.logical_device,
            context.image_available_semaphores[i],
            context.allocator);

        vulkan_fence_destroy(&context, &context.in_flight_fences[i]);
    }

    // Destroy render finished semaphores
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vkDestroySemaphore(context.device.logical_device,
            context.render_finished_semaphores[i],
            context.allocator);

        // Command buffers are already invalidated by the pool reset above
        // Just clear the handles to avoid confusion
        context.graphics_command_buffers[i].handle = nullptr;
    }

    // Clear CAD command buffer handles (already invalidated by pool reset)
    for (u32 i = 0; i < context.swapchain.max_in_flight_frames; ++i) {
        context.cad_command_buffers[i].handle = nullptr;
    }

    // First destroy the Vulkan objects
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.swapchain.framebuffers.data) {
            vulkan_framebuffer_destroy(&context,
                &context.swapchain.framebuffers[i]);
        }
    }

    vulkan_renderpass_destroy(&context, &context.main_renderpass);

    vulkan_swapchain_destroy(&context, &context.swapchain);

    vulkan_device_shutdown(&context);

    vkDestroySurfaceKHR(context.instance, context.surface, context.allocator);

#ifdef DEBUG_BUILD
    CORE_DEBUG("Destroying Vulkan debugger...");
    if (context.debug_messenger) {

        VK_INSTANCE_LEVEL_FUNCTION(context.instance,
            vkDestroyDebugUtilsMessengerEXT);

        vkDestroyDebugUtilsMessengerEXT(context.instance,
            context.debug_messenger,
            context.allocator);
    }
#endif

    vkDestroyInstance(context.instance, context.allocator);

    CORE_DEBUG("Vulkan renderer shut down");
}

void vulkan_on_resized(Renderer_Backend* backend, u16 width, u16 height) {

    cached_framebuffer_width = width;
    cached_framebuffer_height = height;

    ++context.framebuffer_size_generation;

    CORE_INFO("Vulkan renderer backend->resized: w/h/gen: %i %i %llu",
        width,
        height,
        context.framebuffer_size_generation);
}

b8 vulkan_frame_render(Renderer_Backend* backend, f32 delta_t) {

    Vulkan_Device* device = &context.device;

    if (context.recreating_swapchain) {
        // TODO: Blocking operation. To be optimized
        VkResult result = vkDeviceWaitIdle(device->logical_device);

        if (!vulkan_result_is_success(result)) {
            CORE_ERROR("vulkan_begin_frame vkDeviceWaitIdle (1) failed: '%s'",
                vulkan_result_string(result, true));
            return false;
        }

        CORE_INFO("Recreating swapchain, booting.");
        return false;
    }

    // Check if the framebuffer has been resized. If so, a new swapchain
    // must be created and since we will be creating a new swapchain object,
    // we cannot draw a frame image during this frame iteration
    // NOTE:	In renderer_frontend is the begin_frame function returns false
    // 			then the frame is not drawn
    if (context.framebuffer_size_generation !=
        context.framebuffer_size_last_generation) {

        VkResult result = vkDeviceWaitIdle(device->logical_device);
        if (!vulkan_result_is_success(result)) {
            CORE_ERROR("vulkan_begin_frame vkDeviceWaitIdle (2) failed: '%s'",
                vulkan_result_string(result, true));
            return false;
        }

        // If the swapchain recreationg failed (because the windows was
        // minimized) boot out before unsetting the flag
        if (!recreate_swapchain(backend, true)) {
            return false;
        }

        CORE_INFO("Resized, booting.");
        return false;
    }

    // Wait for the execution of the current frame to complete. The frence being
    // free will allow this one to move on
    if (!vulkan_fence_wait(&context,
            &context.in_flight_fences[context.current_frame],
            UINT64_MAX)) {

        CORE_WARN("In-flight fence wait failure!");
        return false;
    }

    // Acquire the next image from the swapchain. Pass along the samephore that
    // should be signaled when this operation completes. This same semaphore
    // will later be waited on by the queue submission to ensure this image is
    // available
    if (!get_next_image_index(backend))
        return false;

    // CORE_DEBUG("frame_render() with frame: '%d' and image index: '%d'",
    //              context.current_frame, context.image_index);

    // At this point we have an image index that we can render to!

    // Begin recording commands
    Vulkan_Command_Buffer* command_buffer =
        &context.graphics_command_buffers[context.image_index];

    vulkan_command_buffer_reset(command_buffer);
    // Mark this command buffer NOT as single use since we are using this over
    // and over again
    vulkan_command_buffer_begin(command_buffer, false, false, false);

    VkViewport viewport;
    // The default viewport of vulkan starts at the top-left corner of the
    // viewport rectangle so coordinates (0; height) instead of (0;0) like in
    // OpenGL. In order to have consistency with other graphics APIs later on,
    // we can offset this.
    viewport.x = 0.0f;
    viewport.y = (f32)context.framebuffer_height;
    viewport.width = (f32)context.framebuffer_width;
    viewport.height = -(f32)context.framebuffer_height;

    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Scissor (basically a Box) clips the scene to the size of the screen
    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context.framebuffer_width;
    scissor.extent.height = context.framebuffer_height;

    vkCmdSetViewport(command_buffer->handle, 0, 1, &viewport);
    vkCmdSetScissor(command_buffer->handle, 0, 1, &scissor);

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;

    // Viewport Pass: Render viewport content to off-screen render target
    vulkan_viewport_render(&context);

    // UI Pass: Begin main renderpass for UI
    vulkan_renderpass_begin(command_buffer,
        &context.main_renderpass,
        context.swapchain.framebuffers[context.image_index].handle);

    // Begin UI frame
    ui_begin_frame();

    return true;
}

b8 vulkan_frame_present(Renderer_Backend* backend, f32 delta_t) {

    Vulkan_Command_Buffer* command_buffer =
        &context.graphics_command_buffers[context.image_index];

    // Render UI and get draw data
    ImDrawData* draw_data = ui_render();
    if (draw_data) {
        // Record ImGui draw commands to the current command buffer
        ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer->handle);
    }

    // End the renderpass
    vulkan_renderpass_end(command_buffer, &context.main_renderpass);

    vulkan_command_buffer_end(command_buffer);

    // Make sure the previous frame is not using this image (i.e. its fence is
    // being waited on)
    if (context.images_in_flight[context.image_index] != nullptr) {
        vulkan_fence_wait(&context,
            context.images_in_flight[context.image_index],
            UINT64_MAX);

        // by the time this operation completes, we are safe to perform ops.
    }

    // CORE_DEBUG("frame_present() with frame: '%d' and image index: '%d'",
    //              context.current_frame, context.image_index);

    // Mark the image fence as in-se by the current frame
    context.images_in_flight[context.image_index] =
        &context.in_flight_fences[context.current_frame];

    vulkan_fence_reset(&context,
        &context.in_flight_fences[context.current_frame]);

    // submit the queue and wait for the operation to complete
    // Begin queue submission
    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer->handle;

    // Semaphores to be signaled when the queue is complete
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores =
        &context.render_finished_semaphores[context.image_index];

    // Wait semaphore ensures that the operation cannot begin until the image
    // is available.
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores =
        &context.image_available_semaphores[context.current_frame];

    // Wait destination stage mask. PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    // which basically prevents the color attachment writes from executing
    // until the semaphore signals. Basically this means that only ONE frame is
    // presented

    VkPipelineStageFlags flags[1] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = flags;

    // All the commands that have been queued will be submitted for execution
    VkResult result =
        vkQueueSubmit(context.device.graphics_queue, // graphics operation
            1,
            &submit_info,
            context.in_flight_fences[context.current_frame].handle);

    if (result != VK_SUCCESS) {
        CORE_ERROR("vkQueueSubmit failed with result: '%s'",
            vulkan_result_string(result, true));
        return false;
    }

    vulkan_command_buffer_update_submitted(command_buffer);

    // Last stage is presentation

    if (!present_frame(backend))
        return false;

    return true;
}

// (TODO) move the check of availability of the required layers outside
// this function
b8 vulkan_enable_validation_layers(
    Auto_Array<const char*>* required_layers_array) {

    CORE_INFO("Vulkan validation layers enabled. Enumerating...");

    // Declare the list of layers that we require
    required_layers_array->push_back("VK_LAYER_KHRONOS_validation");

    // Need to check whether the validation layer requuested is supported
    u32 available_layer_count;

    // To allocate the array needed for storing the available layers,
    // first I need to know how many layers there are
    VK_ENSURE_SUCCESS(
        vkEnumerateInstanceLayerProperties(&available_layer_count, nullptr));

    Auto_Array<VkLayerProperties> available_layers_array;
    available_layers_array.resize(available_layer_count);

    VK_ENSURE_SUCCESS(vkEnumerateInstanceLayerProperties(&available_layer_count,
        available_layers_array.data));

    for (u32 i = 0; i < required_layers_array->length; ++i) {
        CORE_INFO("Searching for layer: %s ...",
            required_layers_array->data[i]);

        b8 found = false;
        for (u32 j = 0; j < available_layer_count; ++j) {
            if (strcmp(required_layers_array->data[i],
                    available_layers_array[j].layerName)) {
                found = true;
                CORE_INFO("Found.");
                break;
            }
        }

        if (!found) {
            CORE_FATAL("Required validation layer is missing: %s",
                required_layers_array->data[i]);
            return false;
        }
    }

    CORE_INFO("All required validaton layers are valid");
    return true;
}

b8 vulkan_create_debug_logger(VkInstance* instance) {

    CORE_DEBUG("Creating Vulkan debug logger");

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

    // Specify the level of events that we want to capture
    debug_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

    // Specify the nature of events that we want to be fed from the validation
    // layer
    debug_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    debug_create_info.pfnUserCallback = vk_debug_callback;

    // Optional pointer that can be passed to the logger. Essentially we can
    // pass whatever data we want and use it in the callback function. Not used
    debug_create_info.pUserData = nullptr;

    // The vkCreateDebugUtilsMessengerEXT is an extension function so it is not
    // loaded automatically. Its address must be looked up manually
    VK_INSTANCE_LEVEL_FUNCTION(*instance, vkCreateDebugUtilsMessengerEXT);

    VK_ENSURE_SUCCESS(vkCreateDebugUtilsMessengerEXT(*instance,
        &debug_create_info,
        context.allocator,
        &context.debug_messenger));

    CORE_DEBUG("Vulkan debugger created");
    return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data) {

    switch (message_severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        CORE_ERROR(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        CORE_WARN(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        CORE_INFO(callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        CORE_TRACE(callback_data->pMessage);
        break;
    default:
        break;
    }
    return VK_FALSE;
}

s32 find_memory_index(u32 type_filter, u32 requested_property_flags) {

    VkPhysicalDeviceMemoryProperties memory_properties;

    // DeviceMemoryProperties structure contains the poroperties of both the
    // device's heaps and its supported memory types. The structure has the
    // memoryTypes[VK_MAX_MEMORY_TYPES] field which is an array of these
    // structures:
    //
    // VkMemoryType {
    // 		VkMemoryPropertyFlags	property_flags;
    // 		uint32_t				heapIndex;
    // }
    //
    // The flags field described the type of memory and is made of a combination
    // of the VkMemoryPropertyFlagBits flags. When creating a Vulka image, the
    // image itself specifies the type of memory it needs on the device in order
    // to be created, so we need to check whether that memory type is supported
    // and if so, we need to return the heapIndex to that memory

    vkGetPhysicalDeviceMemoryProperties(context.device.physical_device,
        &memory_properties);

    for (u32 i = 0; i < memory_properties.memoryTypeCount; ++i) {
        if (
            // Check if memory type i is acceptable according to the type_filter
            // we get from the memory requirements of the image.
            type_filter & (1 << i) &&
            // Check if the memory type i supports all required properties
            // (flags)
            (memory_properties.memoryTypes[i].propertyFlags &
                requested_property_flags) == requested_property_flags) {
            return i;
        }
    }

    CORE_WARN("Memory type not suitable");
    return -1;
}

void create_command_buffers(Renderer_Backend* backend) {
    // For each of our swapchain images, we need to create a command buffer,
    // since the images can be handled asynchronously, so while presenting one
    // image we can draw to the other
    if (!context.graphics_command_buffers.data) {
        context.graphics_command_buffers.resize(context.swapchain.image_count);

        for (u32 i = 0; i < context.swapchain.image_count; ++i) {
            memory_zero(&context.graphics_command_buffers[i],
                sizeof(Vulkan_Command_Buffer));
        }
    }

    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        if (context.graphics_command_buffers[i].handle) {
            vulkan_command_buffer_free(&context,
                context.device.graphics_command_pool,
                &context.graphics_command_buffers[i]);
        }

        memory_zero(&context.graphics_command_buffers[i],
            sizeof(Vulkan_Command_Buffer));

        vulkan_command_buffer_allocate(&context,
            context.device.graphics_command_pool,
            true,
            &context.graphics_command_buffers[i]);
    }

    CORE_DEBUG("Vulkan command buffers created");
}

void create_cad_command_buffers(Vulkan_Context* context) {
    // Create command buffers for CAD off-screen rendering
    // We only need one command buffer per frame in flight for CAD rendering
    if (!context->cad_command_buffers.data) {
        context->cad_command_buffers.resize(context->swapchain.max_in_flight_frames);
        for (u32 i = 0; i < context->swapchain.max_in_flight_frames; ++i) {
            memory_zero(&context->cad_command_buffers[i],
                sizeof(Vulkan_Command_Buffer));
        }
    }

    for (u32 i = 0; i < context->swapchain.max_in_flight_frames; ++i) {
        if (context->cad_command_buffers[i].handle) {
            vulkan_command_buffer_free(context,
                context->device.graphics_command_pool,
                &context->cad_command_buffers[i]);
        }
        memory_zero(&context->cad_command_buffers[i],
            sizeof(Vulkan_Command_Buffer));
        vulkan_command_buffer_allocate(context,
            context->device.graphics_command_pool,
            true,
            &context->cad_command_buffers[i]);
    }
    CORE_DEBUG("CAD command buffers created");
}

// We need a framebuffer per swapchain image
void create_framebuffers(Renderer_Backend* backend,
    Vulkan_Swapchain* swapchain,
    Vulkan_Renderpass* renderpass) {

    for (u32 i = 0; i < swapchain->image_count; ++i) {
        // For now we will have an image view and the depth buffer per image
        u32 attachment_count = 2;

        // Allocate temporarily in stach the attachments, because inside the
        // vulkan_framebuffer_create method we will copy the attachment values
        // in a heap allocated memory
        VkImageView attachments[] = {swapchain->views[i],
            swapchain->depth_attachment.view};

        vulkan_framebuffer_create(&context,
            renderpass,
            context.framebuffer_width,
            context.framebuffer_height,
            attachment_count,
            attachments,
            &swapchain->framebuffers[i]);
    }
}

b8 recreate_swapchain(Renderer_Backend* backend, b8 is_resized_event) {
    if (context.recreating_swapchain) {
        CORE_DEBUG(
            "recreate_swapchain called when already recreating. Booting.");
        return false;
    }

    if (context.framebuffer_width == 0 || context.framebuffer_height == 0) {
        CORE_DEBUG(
            "recreate_swapchain called when window is <1 in a dimension. "
            "Booting.");
        return false;
    }

    if (is_resized_event) {
        CORE_DEBUG("recreate_swapchain triggered due to on_resized event");
    } else {
        CORE_DEBUG("recreate_swapchain triggered due to non-optimal result");
    }

    // Mark as recreating if the dimensions are VALID
    context.recreating_swapchain = true;

    vkDeviceWaitIdle(context.device.logical_device);

    // For saferty, clear these
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        context.images_in_flight[i] = nullptr;
    }

    // Requery support
    vulkan_device_query_swapchain_capabilities(context.device.physical_device,
        context.surface,
        &context.device.swapchain_info);
    vulkan_device_detect_depth_format(&context.device);

    vulkan_swapchain_recreate(&context,
        cached_framebuffer_width,
        cached_framebuffer_height,
        &context.swapchain);

    // Sync the framebuffer size with the cached values, if the size has changed
    if (is_resized_event) {

        // We will have new cached framebuffer sized only if the on_resized
        // event was called, otherwise we need to just recreate the swapchain
        // due to not optimal results of the present or get_next_image operation
        // of the swapchain

        // Ideally we would just want to recreate the swapchain with the new
        // dimension coming from an event of xcb. The problem with that is that
        // the XCB events are asynchronous and can arrive before the Vulkan
        // surface has fully updated internally. This means that although we
        // request a specific width and height, the bounds of the extent could
        // truncate such values, so we must consider the dimensions of the
        // created swapchain (after being truncated with the allowed bounds)
        // instead of the values that we wanted, to prevent inconsistencies
        // context.framebuffer_width = cached_framebuffer_width;
        // context.framebuffer_height = cached_framebuffer_height;

        // Overwrite the framebuffer dimensions to be equal to the swapchain
        context.framebuffer_width = context.swapchain.extent.width;
        context.framebuffer_height = context.swapchain.extent.height;

        context.main_renderpass.w = context.framebuffer_width;
        context.main_renderpass.h = context.framebuffer_height;
        cached_framebuffer_width = 0;
        cached_framebuffer_height = 0;

        context.framebuffer_size_last_generation =
            context.framebuffer_size_generation;
    }

    // Cleanup command buffers
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_command_buffer_free(&context,
            context.device.graphics_command_pool,
            &context.graphics_command_buffers[i]);
    }

    // Destroy framebuffers
    for (u32 i = 0; i < context.swapchain.image_count; ++i) {
        vulkan_framebuffer_destroy(&context,
            &context.swapchain.framebuffers[i]);
    }

    context.main_renderpass.w = context.framebuffer_width;
    context.main_renderpass.h = context.framebuffer_height;
    context.main_renderpass.x = 0;
    context.main_renderpass.y = 0;

    create_framebuffers(backend, &context.swapchain, &context.main_renderpass);

    create_command_buffers(backend);

    // Recreate CAD command buffers
    create_cad_command_buffers(&context);

    context.recreating_swapchain = false;

    CORE_DEBUG("recreate_swapchain completed all operations.");

    return true;
}

b8 get_next_image_index(Renderer_Backend* backend) {

    VkResult result = vkAcquireNextImageKHR(context.device.logical_device,
        context.swapchain.handle,
        UINT64_MAX,
        context.image_available_semaphores[context.current_frame],
        nullptr,
        &context.image_index);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // if (!recreate_swapchain(backend, false))
        //     CORE_FATAL("get_next_image_index failed to recreate swapchain,
        //     due to out-of-date error result");

        return false;

    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        CORE_FATAL("Failed to acquire swapchain iamge!");
        return false;
    }

    return true;
}

b8 present_frame(Renderer_Backend* backend) {

    VkPresentInfoKHR present_info = {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores =
        &context.render_finished_semaphores[context.image_index];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &context.swapchain.handle;
    present_info.pImageIndices = &context.image_index;
    present_info.pResults = 0;

    VkResult result =
        vkQueuePresentKHR(context.device.presentation_queue, &present_info);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {

        // if (!recreate_swapchain(backend, false))
        //     CORE_FATAL("present_frame failed to recreate swapchain after
        //     presentation, due to suboptimal error code");

        // CORE_DEBUG("Swapchain recreated because presentation returned out
        // of date");

    } else if (result != VK_SUCCESS) {
        CORE_FATAL("Failed to present swap chain image!");
        return false;
    }

    context.current_frame =
        (context.current_frame + 1) % context.swapchain.max_in_flight_frames;

    return true;
}

b8 create_ui_library_resources(Vulkan_Context* context) {
    CORE_DEBUG("Creating ImGui Vulkan components...");

    // Create descriptor set layout for ImGui textures
    VkDescriptorSetLayoutBinding binding = {};
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = 1;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.binding = 0;

    VkDescriptorSetLayoutCreateInfo layout_info = {};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 1;
    layout_info.pBindings = &binding;

    VK_ENSURE_SUCCESS(vkCreateDescriptorSetLayout(
        context->device.logical_device,
        &layout_info,
        context->allocator,
        &context->imgui_descriptor_set_layout));

    // Create descriptor pool for ImGui textures
    VkDescriptorPoolSize pool_size = {};
    pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size.descriptorCount = 1000; // Support up to 1000 textures

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = &pool_size;

    VK_ENSURE_SUCCESS(vkCreateDescriptorPool(
        context->device.logical_device,
        &pool_info,
        context->allocator,
        &context->imgui_descriptor_pool));

    // Create linear sampler for ImGui textures
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.minLod = -1000;
    sampler_info.maxLod = 1000;
    sampler_info.maxAnisotropy = 1.0f;

    VK_ENSURE_SUCCESS(vkCreateSampler(
        context->device.logical_device,
        &sampler_info,
        context->allocator,
        &context->imgui_linear_sampler));

    CORE_DEBUG("ImGui Vulkan components created successfully");
    return true;
}

void destroy_ui_library_resources(Vulkan_Context* context) {
    CORE_DEBUG("Destroying ImGui Vulkan components...");

    if (context->imgui_linear_sampler) {
        vkDestroySampler(
            context->device.logical_device,
            context->imgui_linear_sampler,
            context->allocator);
        context->imgui_linear_sampler = VK_NULL_HANDLE;
    }

    if (context->imgui_descriptor_pool) {
        vkDestroyDescriptorPool(
            context->device.logical_device,
            context->imgui_descriptor_pool,
            context->allocator);
        context->imgui_descriptor_pool = VK_NULL_HANDLE;
    }

    if (context->imgui_descriptor_set_layout) {
        vkDestroyDescriptorSetLayout(
            context->device.logical_device,
            context->imgui_descriptor_set_layout,
            context->allocator);
        context->imgui_descriptor_set_layout = VK_NULL_HANDLE;
    }

    CORE_DEBUG("ImGui Vulkan components destroyed");
}

Vulkan_Context* vulkan_get_context() {
    return &context;
}

VkDescriptorSet vulkan_get_cad_texture() {
    return vulkan_viewport_get_texture(&context);
}

void vulkan_resize_cad_render_target(u32 width, u32 height) {
    vulkan_viewport_resize(&context, width, height);
}
