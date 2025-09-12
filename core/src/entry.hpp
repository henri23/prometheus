#pragma once

#include "defines.hpp"
#include "stdio.h"
#include "application/application.hpp"

extern b8 create_client();

int main() {
  if (!create_client()) {
    return -1;
  }

  printf("Test");

  return 0;
}
