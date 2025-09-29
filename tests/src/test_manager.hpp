#pragma once
#include "defines.hpp"

#define BYPASS 2

// The test functions can either return 1 for success, or 2 for bypass. 
// Any other return value will be considered as failure
typedef u8 (*PFN_test)();

void test_manager_init();
void test_manager_register_test(PFN_test, const char* desc);
void test_manager_run_tests();

