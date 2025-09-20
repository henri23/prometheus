#pragma once

#include "core/application.hpp"
#include "defines.hpp"

// Forward declaration for client UI config
typedef struct Client_UI_Config Client_UI_Config;

extern Client_UI_Config* create_client();

int main() {
    Client_UI_Config* client_config = create_client();
    if (!client_config) {
        return -1;
    }

    App_Config config = application_get_default_config();
    if (!application_init(&config)) {
        return -1;
    }

    // Set client UI configuration for the application
    if (!application_set_client_ui_config(client_config)) {
        return -1;
    }

    application_run();

    return 0;
}
