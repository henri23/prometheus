#pragma once

#include "defines.hpp"

// b8 platform_startup(u64* mem_req,
// 					void* state,
//                     const char* application_name,
//                     s32 x,
//                     s32 y,
//                     s32 width,
//                     s32 height);
//
// void platform_shutdown(void* state);
//
// b8 platform_message_pump();

void* platform_allocate(u64 size, b8 aligned);

void platform_free(void* block, b8 aligned);

void* platform_zero_memory(void* block, u64 size);

void* platform_copy_memory(void* dest, const void* source, u64 size);

void* platform_move_memory(void* dest, const void* source, u64 size);

void* platform_set_memory(void* dest, s32 value, u64 size);

f64 platform_get_absolute_time();

void platform_sleep(u64 ms);
