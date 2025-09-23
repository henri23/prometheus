#pragma once

#include "defines.hpp"
#include "imgui.h"

// 2D viewport state for CAD workspace
struct Viewport_State {
    // View transformation
    ImVec2 pan_offset;     // Current pan offset in world space
    f32 zoom_level;        // Current zoom level (1.0 = 100%)

    // Grid settings
    b8 show_grid;          // Whether to show the grid
    f32 grid_size;         // Grid spacing in world units
    f32 grid_subdivisions; // Number of subdivisions per major grid line

    // Viewport bounds
    ImVec2 viewport_pos;   // Position of viewport in screen space
    ImVec2 viewport_size;  // Size of viewport in screen space

    // Interaction state
    b8 is_panning;         // Currently panning
    b8 is_zooming;         // Currently zooming
    ImVec2 last_mouse_pos; // Last mouse position for delta calculations

    // Visual settings
    ImU32 grid_color;      // Grid line color
    ImU32 grid_major_color;// Major grid line color
    ImU32 background_color;// Viewport background color
};

// Initialize viewport layer
b8 ui_viewport_initialize();

// Shutdown viewport layer
void ui_viewport_shutdown();

// Draw the viewport (called from UI layer)
void ui_viewport_draw(void* component_state);

// Viewport coordinate transformations
ImVec2 ui_viewport_world_to_screen(const Viewport_State* viewport, ImVec2 world_pos);
ImVec2 ui_viewport_screen_to_world(const Viewport_State* viewport, ImVec2 screen_pos);

// Viewport controls
void ui_viewport_pan(Viewport_State* viewport, ImVec2 delta);
void ui_viewport_zoom(Viewport_State* viewport, f32 zoom_delta, ImVec2 zoom_center);
void ui_viewport_reset_view(Viewport_State* viewport);

// Grid drawing
void ui_viewport_draw_grid(const Viewport_State* viewport, ImDrawList* draw_list);