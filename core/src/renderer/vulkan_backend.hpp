#pragma once

#include "defines.hpp"

b8 renderer_initialize(struct Platform_State* plat_state);

void renderer_shutdown();

b8 renderer_frame_render();

b8 vulkan_frame_present();
