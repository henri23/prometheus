#include "input_codes.hpp"

// Since our engine codes match SDL3/USB HID standard, conversion is simple
u32 key_code_to_platform(Key_Code key) {
    return (u32)key;
}

Key_Code platform_to_key_code(u32 platform_key) {
    if (platform_key >= (u32)Key_Code::MAX_KEYS) {
        return Key_Code::UNKNOWN;
    }
    return (Key_Code)platform_key;
}

u8 mouse_button_to_platform(Mouse_Button button) {
    return (u8)button;
}

Mouse_Button platform_to_mouse_button(u8 platform_button) {
    if (platform_button >= (u8)Mouse_Button::MAX_BUTTONS) {
        return Mouse_Button::UNKNOWN;
    }
    return (Mouse_Button)platform_button;
}