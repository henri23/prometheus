#pragma once

#include "defines.hpp"

// Client configuration structure - client controls engine behavior
struct App_Config {
    const char* name;
    u32 width;
    u32 height;
    const char* icon_path;
    b8 window_resizable;
    b8 custom_titlebar;
    b8 use_dockspace;
    b8 center_window;
};

// Client state structure - similar to Game struct in koala_engine
struct Client_State {
    App_Config config;

    // Lifecycle callbacks - client implements these
    b8 (*initialize)(Client_State*);
    b8 (*update)(Client_State*, f32 delta_time);
    b8 (*render)(Client_State*, f32 delta_time);
    void (*on_resize)(Client_State*, u32 width, u32 height);
    void (*shutdown)(Client_State*);

    // Client-specific state (managed by client)
    void* state;

    // Internal engine state (opaque pointer managed by core)
    // Client cannot access this directly - only core can use this
    void* application_state;
};
