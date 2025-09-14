#pragma once

#include "containers/auto_array.hpp"
#include "vulkan_types.hpp"

void platform_get_vulkan_extensions(Auto_Array<const char*>* extensions);

b8 platform_create_vulkan_surface(
    struct Platform_State* plat_state,
    Vulkan_Context* context);

b8 platform_get_window_details(
	struct Platform_State* plat_state,
	u32* width,
	u32* height,
	f32* main_scale);
