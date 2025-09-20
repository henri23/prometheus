#pragma once

#include "defines.hpp"

#define ASSERTIONS_ENABLED

// Based on the platform the instruction that stops debugging will be different
#if PLATFORM_WINDOWS
#include <intrin.h>
#define debug_break() __debugbreak()
#else
// Forward declare the builtin function for template contexts
extern "C" void __builtin_debugtrap(void);
#define debug_break() __builtin_debugtrap()  // Stops the applicaiton in a way that the debugger can catch it
#endif

// Need to export even though we are calling the macro
// The editor/game must be able to access these functionalities
// NOTE: Implemented in logger.cpp to keep the logging functionalities in one place
PROMETHEUS_API void report_assertion_failure(
    const char* expression,
    const char* message,
    const char* file,
    s32 line);

#ifdef ASSERTIONS_ENABLED

#define RUNTIME_ASSERT_MSG(expr, message)             \
    if (expr) {                                       \
    } else {                                          \
        report_assertion_failure(#expr, message,      \
                                 __FILE__, __LINE__); \
        debug_break();                                \
    }

#define RUNTIME_ASSERT(expr) RUNTIME_ASSERT_MSG(expr, "") 

#else
#define RUNTIME_ASSERT_MSG(expr, message) 
#define RUNTIME_ASSERT(expr) 
#endif

