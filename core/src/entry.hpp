#pragma once

#include "defines.hpp"
#include "stdio.h"
#include "core/application.hpp"
#include "core/logger.hpp"

extern b8 create_client();

int main() {
  if (!create_client()) {
    return -1;
  }

  application_init();

  application_run();

  return 0;
}
