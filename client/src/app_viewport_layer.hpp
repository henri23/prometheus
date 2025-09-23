#pragma once

#include "ui/ui.hpp"

// Example application layer that demonstrates the viewport
b8 app_viewport_layer_initialize();
void app_viewport_layer_shutdown();
void app_viewport_layer_render(void* component_state);