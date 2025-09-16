#pragma once

#include "defines.hpp"

// Forward declarations
struct ImDrawData;

b8 renderer_initialize();

void renderer_shutdown();

b8 renderer_draw_frame(ImDrawData* draw_data);

// Vulkan backend initialization for ImGui (called by UI subsystem)
b8 renderer_init_imgui_vulkan();

// Get SDL window for ImGui initialization (called by UI subsystem)
void* renderer_get_sdl_window();

// Wait for all GPU operations to complete (for safe shutdown)
b8 renderer_wait_idle();
