#pragma once

#include "defines.hpp"

struct App_Config {

};

PROMETHEUS_API b8 application_init();

PROMETHEUS_API b8 application_run();

void application_shutdown();
