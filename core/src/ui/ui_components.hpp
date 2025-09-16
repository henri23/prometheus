#pragma once

#include "defines.hpp"

// Forward declarations
struct UI_State;

/**
 * UI Components Module
 * Contains all ImGui UI component definitions
 * Provides extensible system for adding new UI components
 */

/**
 * Render the demo window component
 * @param ui_state - UI state containing show flags
 */
void ui_render_demo_window(UI_State* ui_state);

/**
 * Render the main Prometheus window component
 * @param ui_state - UI state containing show flags
 */
void ui_render_prometheus_window(UI_State* ui_state);

/**
 * Render performance information window
 * @param ui_state - UI state containing show flags
 */
void ui_render_performance_window(UI_State* ui_state);

/**
 * Render all active UI components
 * This is the main entry point for component rendering
 * @param ui_state - UI state containing show flags
 */
void ui_render_all_components(UI_State* ui_state);