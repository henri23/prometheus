#include "renderer_backend.hpp"

#include "containers/auto_array.hpp"
#include "core/logger.hpp"
#include "imgui.h"
#include "memory/memory.hpp"
#include "platform/platform.hpp"
#include "renderer_platform.hpp"
#include "ui/ui.hpp"
#include "ui/ui_themes.hpp"

// Renderer configuration
// #define UNLIMITED_FRAME_RATE  // Uncomment for software frame limiting (may
// cause tearing)

internal_variable Vulkan_Context context;

// Vulkan configuration
internal_variable u32 g_MinImageCount = 2;

// Forward declare messenger callback
VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_types,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data);

// Forward declare internal functions
b8 renderer_frame_render(ImDrawData* draw_data);
b8 renderer_frame_present();

INTERNAL_FUNC bool is_extension_available(
    const Auto_Array<VkExtensionProperties>& properties,
    const char* extension) {

    for (const VkExtensionProperties& p : properties)
        if (strcmp(p.extensionName, extension) == 0)
            return true;

    return false;
}

INTERNAL_FUNC void setup_vulkan_window(ImGui_ImplVulkanH_Window* wd,
    VkSurfaceKHR surface,
    u32 width,
    u32 height) {

    wd->Surface = surface;

    // Check for WSI support
    VkBool32 res;

    vkGetPhysicalDeviceSurfaceSupportKHR(context.physical_device,
        context.queue_family,
        wd->Surface,
        &res);

    if (res != VK_TRUE) {
        RUNTIME_ASSERT_MSG(false, "Error no WSI support on physical device 0");
    }

    // Select Surface Format
    const VkFormat requestSurfaceImageFormat[] = {
        VK_FORMAT_R8G8B8A8_UNORM, // Prefer RGBA over BGRA to avoid red tint
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_FORMAT_R8G8B8_UNORM,
        VK_FORMAT_B8G8R8_UNORM};

    const VkColorSpaceKHR requestSurfaceColorSpace =
        VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    wd->SurfaceFormat =
        ImGui_ImplVulkanH_SelectSurfaceFormat(context.physical_device,
            wd->Surface,
            requestSurfaceImageFormat,
            (size_t)IM_ARRAYSIZE(requestSurfaceImageFormat),
            requestSurfaceColorSpace);

    // Select Present Mode based on configuration
#ifdef UNLIMITED_FRAME_RATE
    // Prioritize unlimited frame rate modes (software frame limiting)
    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_IMMEDIATE_KHR, // No VSync - allows software frame
                                       // limiting
        VK_PRESENT_MODE_MAILBOX_KHR,   // VSync with triple buffering (fallback)
        VK_PRESENT_MODE_FIFO_KHR};     // VSync (fallback)
#else
    // Prioritize VSync modes for tear-free rendering (default)
    VkPresentModeKHR present_modes[] = {
        VK_PRESENT_MODE_MAILBOX_KHR,    // VSync with triple buffering (smooth)
        VK_PRESENT_MODE_FIFO_KHR,       // VSync (guaranteed available)
        VK_PRESENT_MODE_IMMEDIATE_KHR}; // No VSync (fallback only)
#endif

    wd->PresentMode =
        ImGui_ImplVulkanH_SelectPresentMode(context.physical_device,
            wd->Surface,
            &present_modes[0],
            IM_ARRAYSIZE(present_modes));

    // Log which present mode was selected
    const char* present_mode_name = "Unknown";
    switch (wd->PresentMode) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
        present_mode_name = "IMMEDIATE (No VSync)";
        break;
    case VK_PRESENT_MODE_MAILBOX_KHR:
        present_mode_name = "MAILBOX (VSync + Triple Buffer)";
        break;
    case VK_PRESENT_MODE_FIFO_KHR:
        present_mode_name = "FIFO (VSync)";
        break;
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
        present_mode_name = "FIFO_RELAXED (Adaptive VSync)";
        break;
	default:
        present_mode_name = "*invalid presentation mode*";
		break;
    }

    CORE_INFO("Selected Vulkan present mode: %s", present_mode_name);
    CORE_INFO("Swapchain image count: %u", wd->ImageCount);

    // Create SwapChain, RenderPass, Framebuffer, etc.
    RUNTIME_ASSERT(g_MinImageCount >= 2);
    ImGui_ImplVulkanH_CreateOrResizeWindow(context.instance,
        context.physical_device,
        context.logical_device,
        wd,
        context.queue_family,
        context.allocator,
        width,
        height,
        g_MinImageCount);
}

// Function to initialize Vulkan backend for ImGui
// This will be called by the UI subsystem during initialization
b8 renderer_init_imgui_vulkan() {
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = context.instance;
    init_info.PhysicalDevice = context.physical_device;
    init_info.Device = context.logical_device;
    init_info.QueueFamily = context.queue_family;
    init_info.Queue = context.queue;
    init_info.PipelineCache = context.pipeline_cache;
    init_info.DescriptorPool = context.descriptor_pool;
    init_info.RenderPass = context.main_window_data.RenderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = g_MinImageCount;
    init_info.ImageCount = context.main_window_data.ImageCount;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = context.allocator;
    init_info.CheckVkResultFn = nullptr;

    return ImGui_ImplVulkan_Init(&init_info);
}

b8 renderer_initialize() {

    // TODO: For now do not use custom allocator. Maybe change later...
    context.allocator = nullptr;

    // Create Vulkan Instance
    {
        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        // Enumerate available extensions
        uint32_t properties_count;
        Auto_Array<VkExtensionProperties> properties;

        vkEnumerateInstanceExtensionProperties(nullptr,
            &properties_count,
            nullptr);

        properties.resize(properties_count);

        VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr,
            &properties_count,
            properties.data));

        // Enable required extensions
        Auto_Array<const char*> instance_extensions;

        platform_get_vulkan_extensions(&instance_extensions);

        if (is_extension_available(properties,
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME))

            instance_extensions.push_back(
                VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

#ifdef VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
        if (is_extension_available(properties,
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME)) {

            instance_extensions.push_back(
                VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

            create_info.flags |=
                VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        }
#endif

        // Enabling validation layers
#ifdef DEBUG_BUILD
        // Check if validation layers are available
        uint32_t layer_count;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        Auto_Array<VkLayerProperties> available_layers;
        available_layers.resize(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data);

        bool validation_layer_found = false;
        for (const auto& layer : available_layers) {
            if (strcmp(layer.layerName, "VK_LAYER_KHRONOS_validation") == 0) {
                validation_layer_found = true;
                break;
            }
        }

        if (validation_layer_found) {
            const char* layers[] = {"VK_LAYER_KHRONOS_validation"};
            create_info.enabledLayerCount = 1;
            create_info.ppEnabledLayerNames = layers;
            instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
            CORE_DEBUG("Vulkan validation layers enabled");
        } else {
            CORE_WARN("Vulkan validation layers not available");
        }
#endif

        // Create Vulkan Instance
        create_info.enabledExtensionCount = (u32)instance_extensions.length;
        create_info.ppEnabledExtensionNames = instance_extensions.data;

        VK_CHECK(vkCreateInstance(&create_info,
            context.allocator,
            &context.instance));

        // Setup the debug report callback
#ifdef DEBUG_BUILD
        if (validation_layer_found) {
            VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {
                VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};

            // Specify the level of events that we want to capture
            debug_create_info.messageSeverity =
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;

            // Specify the nature of events that we want to be fed from the
            // validation layer
            debug_create_info.messageType =
                VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

            debug_create_info.pfnUserCallback = vk_debug_callback;

            // Optional pointer that can be passed to the logger. Essentially we
            // can pass whatever data we want and use it in the callback
            // function. Not used
            debug_create_info.pUserData = nullptr;

            // Enable additional validation features
            VkValidationFeatureEnableEXT validation_features[] = {
                VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
                VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
                VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT};

            VkValidationFeaturesEXT validation_features_ext = {};
            validation_features_ext.sType =
                VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
            validation_features_ext.enabledValidationFeatureCount =
                sizeof(validation_features) / sizeof(validation_features[0]);

            validation_features_ext.pEnabledValidationFeatures =
                validation_features;

            debug_create_info.pNext = &validation_features_ext;

            // The vkCreateDebugUtilsMessengerEXT is an extension function so it
            // is not loaded automatically. Its address must be looked up
            // manually
            VK_INSTANCE_LEVEL_FUNCTION(context.instance,
                vkCreateDebugUtilsMessengerEXT);

            VK_CHECK(vkCreateDebugUtilsMessengerEXT(context.instance,
                &debug_create_info,
                context.allocator,
                &context.debug_messenger));

            CORE_DEBUG("Vulkan debugger created");
        }
#endif
    }

    // Select Physical Device (GPU)
    // Select GPU
    {
        u32 gpu_count;
        VK_CHECK(
            vkEnumeratePhysicalDevices(context.instance, &gpu_count, nullptr));

        RUNTIME_ASSERT(gpu_count > 0);

        VkPhysicalDevice* gpus = (VkPhysicalDevice*)memory_allocate(
            sizeof(VkPhysicalDevice) * gpu_count,
            Memory_Tag::RENDERER);

        VK_CHECK(
            vkEnumeratePhysicalDevices(context.instance, &gpu_count, gpus));

        // If a number >1 of GPUs got reported, find discrete GPU if present,
        // or use first one available. This covers most common cases
        // (multi-gpu/integrated+dedicated graphics). Handling more complicated
        // setups (multiple dedicated GPUs) is out of scope of this sample.
        u32 use_gpu = 0;
        for (u32 i = 0; i < gpu_count; i++) {

            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(gpus[i], &properties);

            if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {

                use_gpu = i;
                break;
            }
        }

        context.physical_device = gpus[use_gpu];
        memory_deallocate(gpus,
            sizeof(VkPhysicalDevice) * gpu_count,
            Memory_Tag::RENDERER);
    }

    RUNTIME_ASSERT(context.physical_device != VK_NULL_HANDLE);

    // Select graphics queue family
    context.queue_family =
        ImGui_ImplVulkanH_SelectQueueFamilyIndex(context.physical_device);

    RUNTIME_ASSERT(context.queue_family != (u32)-1);

    // Create Logical Device (with 1 queue)
    {
        Auto_Array<const char*> device_extensions;
        device_extensions.push_back("VK_KHR_swapchain");

        // Enumerate physical device extension
        uint32_t properties_count;
        Auto_Array<VkExtensionProperties> properties;

        vkEnumerateDeviceExtensionProperties(context.physical_device,
            nullptr,
            &properties_count,
            nullptr);

        properties.resize(properties_count);

        vkEnumerateDeviceExtensionProperties(context.physical_device,
            nullptr,
            &properties_count,
            properties.data);

        const float queue_priority[] = {1.0f};

        VkDeviceQueueCreateInfo queue_info[1] = {};
        queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info[0].queueFamilyIndex = context.queue_family;
        queue_info[0].queueCount = 1;
        queue_info[0].pQueuePriorities = queue_priority;

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount =
            sizeof(queue_info) / sizeof(queue_info[0]);
        create_info.pQueueCreateInfos = queue_info;
        create_info.enabledExtensionCount = (u32)device_extensions.length;
        create_info.ppEnabledExtensionNames = device_extensions.data;

        VK_CHECK(vkCreateDevice(context.physical_device,
            &create_info,
            context.allocator,
            &context.logical_device));

        CORE_DEBUG("Vulkan logical device created.");

        vkGetDeviceQueue(context.logical_device,
            context.queue_family,
            0,
            &context.queue);
    }

    // Create Descriptor Pool
    // If you wish to load e.g. additional textures you may need to alter
    // pools sizes and maxSets.
    {
        VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
            {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
            {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
            {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (u32)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;

        VK_CHECK(vkCreateDescriptorPool(context.logical_device,
            &pool_info,
            context.allocator,
            &context.descriptor_pool));

        CORE_DEBUG("Descriptor pool created.");
    }

    // Create window surface
    if (!platform_create_vulkan_surface(&context)) {
        CORE_ERROR("Error while creating vulkan surface.");
        return false;
    }

    u32 width, height;
    f32 main_scale;

    if (!platform_get_window_details(&width, &height, &main_scale)) {
        CORE_ERROR("Error while retrieving window details");
        return false;
    }

    ImGui_ImplVulkanH_Window* wd = &context.main_window_data;

    setup_vulkan_window(wd, context.surface, width, height);

    CORE_INFO("Vulkan renderer initialized successfully");
    return true;
}

void renderer_shutdown() {
    CORE_DEBUG("Shutting down renderer...");

    VK_CHECK(vkDeviceWaitIdle(context.logical_device));

    // Note: ImGui shutdown is handled by UI subsystem

    ImGui_ImplVulkanH_DestroyWindow(context.instance,
        context.logical_device,
        &context.main_window_data,
        context.allocator);

    vkDestroyDescriptorPool(context.logical_device,
        context.descriptor_pool,
        context.allocator);

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

    vkDestroyDevice(context.logical_device, context.allocator);
    vkDestroyInstance(context.instance, context.allocator);

    CORE_DEBUG("Renderer shut down.");
}

b8 renderer_wait_idle() {
    if (context.logical_device == VK_NULL_HANDLE) {
        return false;
    }

    VkResult result = vkDeviceWaitIdle(context.logical_device);
    if (result != VK_SUCCESS) {
        CORE_ERROR("Failed to wait for device idle: VkResult = %d", result);
        return false;
    }

    return true;
}

void renderer_trigger_swapchain_recreation() {
    context.swapchain_rebuild = true;
}


VkDevice renderer_get_device() { return context.logical_device; }

VkPhysicalDevice renderer_get_physical_device() {
    return context.physical_device;
}

VkQueue renderer_get_queue() { return context.queue; }

VkAllocationCallbacks* renderer_get_allocator() { return context.allocator; }

VkCommandPool renderer_get_command_pool() {
    return context.main_window_data.Frames[context.main_window_data.FrameIndex]
        .CommandPool;
}

// Clear color is now determined by the current UI theme

b8 renderer_draw_frame(ImDrawData* draw_data) {
    // Handle window resize
    u32 fb_width, fb_height;
    f32 not_important;

    platform_get_window_details(&fb_width, &fb_height, &not_important);

    if (fb_width > 0 && fb_height > 0 &&
        (context.swapchain_rebuild ||
            context.main_window_data.Width != fb_width ||
            context.main_window_data.Height != fb_height)) {

        ImGui_ImplVulkan_SetMinImageCount(g_MinImageCount);
        RUNTIME_ASSERT(g_MinImageCount >= 2);
        ImGui_ImplVulkanH_CreateOrResizeWindow(context.instance,
            context.physical_device,
            context.logical_device,
            &context.main_window_data,
            context.queue_family,
            context.allocator,
            fb_width,
            fb_height,
            g_MinImageCount);

        context.main_window_data.FrameIndex = 0;
        context.swapchain_rebuild = false;
    }

    // Only render if we have valid draw data (not minimized)
    if (draw_data && draw_data->DisplaySize.x > 0.0f &&
        draw_data->DisplaySize.y > 0.0f) {

        // Set clear color based on current UI theme
        ImVec4 theme_clear_color =
            ui_themes_get_clear_color(ui_get_current_theme());
        context.main_window_data.ClearValue.color.float32[0] =
            theme_clear_color.x * theme_clear_color.w;
        context.main_window_data.ClearValue.color.float32[1] =
            theme_clear_color.y * theme_clear_color.w;
        context.main_window_data.ClearValue.color.float32[2] =
            theme_clear_color.z * theme_clear_color.w;
        context.main_window_data.ClearValue.color.float32[3] =
            theme_clear_color.w;

        renderer_frame_render(draw_data);
        renderer_frame_present();
    }

    return true;
}

b8 renderer_frame_render(ImDrawData* draw_data) {
    auto wd = &context.main_window_data;

    VkSemaphore image_acquired_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].ImageAcquiredSemaphore;

    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

    VkResult err = vkAcquireNextImageKHR(context.logical_device,
        wd->Swapchain,
        UINT64_MAX,
        image_acquired_semaphore,
        VK_NULL_HANDLE,
        &wd->FrameIndex);

    if (err == VK_ERROR_OUT_OF_DATE_KHR) {
        context.swapchain_rebuild = true;
        return true; // Skip this frame and recreate swapchain on next frame
    }
    if (err != VK_SUCCESS && err != VK_SUBOPTIMAL_KHR) {
        CORE_ERROR("Failed to acquire swapchain image, error: %d", err);
        return false;
    }

    ImGui_ImplVulkanH_Frame* fd = &wd->Frames[wd->FrameIndex];
    {
        // wait indefinitely instead of periodically checking
        VK_CHECK(vkWaitForFences(context.logical_device,
            1,
            &fd->Fence,
            VK_TRUE,
            UINT64_MAX));

        VK_CHECK(vkResetFences(context.logical_device, 1, &fd->Fence));
    }
    {
        VK_CHECK(
            vkResetCommandPool(context.logical_device, fd->CommandPool, 0));

        VkCommandBufferBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        VK_CHECK(vkBeginCommandBuffer(fd->CommandBuffer, &info));
    }
    {
        VkRenderPassBeginInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        info.renderPass = wd->RenderPass;
        info.framebuffer = fd->Framebuffer;
        info.renderArea.extent.width = wd->Width;
        info.renderArea.extent.height = wd->Height;
        info.clearValueCount = 1;
        info.pClearValues = &wd->ClearValue;
        vkCmdBeginRenderPass(fd->CommandBuffer,
            &info,
            VK_SUBPASS_CONTENTS_INLINE);
    }

    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, fd->CommandBuffer);

    // Submit command buffer
    vkCmdEndRenderPass(fd->CommandBuffer);
    {
        VkPipelineStageFlags wait_stage =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &image_acquired_semaphore;
        info.pWaitDstStageMask = &wait_stage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &fd->CommandBuffer;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &render_complete_semaphore;

        VK_CHECK(vkEndCommandBuffer(fd->CommandBuffer));

        VK_CHECK(vkQueueSubmit(context.queue, 1, &info, fd->Fence));
    }
    return true;
}

b8 renderer_frame_present() {

    auto wd = &context.main_window_data;

    if (context.swapchain_rebuild)
        return false;

    VkSemaphore render_complete_semaphore =
        wd->FrameSemaphores[wd->SemaphoreIndex].RenderCompleteSemaphore;

    VkPresentInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &render_complete_semaphore;
    info.swapchainCount = 1;
    info.pSwapchains = &wd->Swapchain;
    info.pImageIndices = &wd->FrameIndex;

    VkResult err = vkQueuePresentKHR(context.queue, &info);

    if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
        context.swapchain_rebuild = true;
    if (err == VK_ERROR_OUT_OF_DATE_KHR)
        return false;
    if (err != VK_SUBOPTIMAL_KHR) {
        VK_CHECK(err);
    }

    // Now we can use the next set of semaphores
    wd->SemaphoreIndex = (wd->SemaphoreIndex + 1) % wd->SemaphoreCount;

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
