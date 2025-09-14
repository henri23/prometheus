#pragma once

#include "core/application.hpp"
#include "defines.hpp"

extern b8 create_client();

int main() {
    if (!create_client()) {
        return -1;
    }

    if (!application_init()) {
        return -1;
    }

    application_run();

    return 0;
}
