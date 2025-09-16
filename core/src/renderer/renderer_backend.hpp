#pragma once

#include "defines.hpp"

b8 renderer_initialize();

void renderer_shutdown();

b8 renderer_draw_frame(b8* show_demo);
