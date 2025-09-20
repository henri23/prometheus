#include <entry.hpp>
#include "ui/client_ui.hpp"

Client_UI_Config* create_client() {
    // Initialize client UI system
    if (!client_ui_initialize()) {
        return nullptr;
    }

    // Return client UI configuration for core to use
    return client_ui_get_config();
}
