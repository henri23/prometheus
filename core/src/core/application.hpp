#pragma once

#include "defines.hpp"

// Forward declaration for client state (defined in client_types.hpp)
struct Client;

// Initialize application with client state
PROMETHEUS_API b8 application_init(Client* client_state);

// Run the main application loop
PROMETHEUS_API void application_run();

// Shutdown and cleanup application
void application_shutdown();
