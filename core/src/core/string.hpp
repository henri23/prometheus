#pragma once

#include "defines.hpp"
#include <stdarg.h>

PROMETHEUS_API b8 string_check_equal(
    const char* str1,
    const char* str2);

PROMETHEUS_API s32 string_format(
	char* dest, 
	const char* format, ...);

PROMETHEUS_API s32 string_format_v(
    char* dest,
    const char* format,
    va_list va_list);

PROMETHEUS_API u64 string_length(
    const char* string);
