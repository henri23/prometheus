#include "ui_fonts.hpp"

#include "assets/assets.hpp"
#include "containers/auto_array.hpp"
#include "core/asserts.hpp"
#include "core/logger.hpp"
#include "imgui.h"
#include "memory/memory.hpp"
#include <cstring>

// Font information structure (internal only)
struct UI_Font_Info {
    const char* name;
    const char* family;
    UI_Font_Weight weight;
    UI_Font_Style style;
    f32 size;
    const u8* data;
    u32 data_size;
    ImFont* imgui_font;
    b8 is_loaded;
    b8 is_default;

    // Equality operator for Auto_Array find functionality
    bool operator==(const UI_Font_Info& other) const {
        return name && other.name && strcmp(name, other.name) == 0;
    }
};

// Font registry (internal only)
struct UI_Font_Registry {
    Auto_Array<UI_Font_Info> fonts;
    ImFont* default_font;
    b8 is_initialized;
};

// Internal font system state
internal_variable UI_Font_Registry font_registry = {};

// Default embedded font data (placeholder - in real implementation would be actual font data)
// For now, we'll use ImGui's default font
internal_variable const u8 default_font_data[] = {0}; // Placeholder
internal_variable const u32 default_font_data_size = 0;

// Internal functions
INTERNAL_FUNC UI_Font_Info* find_font_by_name(const char* name);
INTERNAL_FUNC ImFontConfig create_font_config(const UI_Font_Info* font_info);

b8 ui_fonts_initialize() {
    CORE_DEBUG("Initializing font management system...");

    if (font_registry.is_initialized) {
        CORE_WARN("Font system already initialized");
        return true;
    }

    // Initialize font registry (Auto_Array handles its own memory)
    font_registry.fonts.clear();
    font_registry.default_font = nullptr;
    font_registry.is_initialized = true;

    CORE_INFO("Font management system initialized");
    return true;
}

void ui_fonts_shutdown() {
    CORE_DEBUG("Shutting down font management system...");

    if (!font_registry.is_initialized) {
        CORE_WARN("Font system not initialized - already shut down or never initialized");
        return;
    }

    // Note: ImGui fonts are managed by ImGui's font atlas
    // Auto_Array destructor handles memory cleanup automatically

    // Reset registry
    font_registry = {};

    CORE_DEBUG("Font management system shut down successfully");
}

b8 ui_fonts_register_embedded(const char* name, const char* family, UI_Font_Weight weight, UI_Font_Style style, const u8* data, u32 data_size, f32 size) {
    RUNTIME_ASSERT_MSG(name, "Font name cannot be null");
    RUNTIME_ASSERT_MSG(family, "Font family cannot be null");
    // Note: data can be null for default font
    if (data != nullptr) {
        RUNTIME_ASSERT_MSG(data_size > 0, "Font data size must be positive when data is provided");
    }

    if (!font_registry.is_initialized) {
        CORE_ERROR("Font system not initialized");
        return false;
    }

    // Check if font already exists
    if (find_font_by_name(name)) {
        CORE_WARN("Font '%s' already registered", name);
        return false;
    }

    // Create and add font info
    UI_Font_Info font_info = {
        .name = name,
        .family = family,
        .weight = weight,
        .style = style,
        .size = size,
        .data = data,
        .data_size = data_size,
        .imgui_font = nullptr,
        .is_loaded = false,
        .is_default = false};

    font_registry.fonts.push_back(font_info);

    CORE_DEBUG("Registered embedded font: %s (%.1fpt)", name, size);
    return true;
}

b8 ui_fonts_register_system(const char* name, const char* family, UI_Font_Weight weight, UI_Font_Style style, const char* filepath, f32 size) {
    RUNTIME_ASSERT_MSG(name, "Font name cannot be null");
    RUNTIME_ASSERT_MSG(family, "Font family cannot be null");
    RUNTIME_ASSERT_MSG(filepath, "Font filepath cannot be null");

    if (!font_registry.is_initialized) {
        CORE_ERROR("Font system not initialized");
        return false;
    }

    // Check if font already exists
    if (find_font_by_name(name)) {
        CORE_WARN("Font '%s' already registered", name);
        return false;
    }

    // Create and add font info for system font
    UI_Font_Info font_info = {
        .name = name,
        .family = family,
        .weight = weight,
        .style = style,
        .size = size,
        .data = (const u8*)filepath, // Store filepath for system fonts
        .data_size = 0,              // 0 indicates system font
        .imgui_font = nullptr,
        .is_loaded = false,
        .is_default = false};

    font_registry.fonts.push_back(font_info);

    CORE_DEBUG("Registered system font: %s -> %s (%.1fpt)", name, filepath, size);
    return true;
}

b8 ui_fonts_load_all() {
    if (!font_registry.is_initialized) {
        CORE_ERROR("Font system not initialized");
        return false;
    }

    CORE_DEBUG("Loading all registered fonts into ImGui...");

    ImGuiIO& io = ImGui::GetIO();

    // Load each registered font
    for (u32 i = 0; i < font_registry.fonts.size(); ++i) {
        UI_Font_Info* font_info = &font_registry.fonts[i];

        if (font_info->is_loaded) {
            continue; // Skip already loaded fonts
        }

        ImFontConfig config = create_font_config(font_info);

        if (font_info->data && font_info->data_size > 0) {
            // Load embedded font from memory
            font_info->imgui_font = io.Fonts->AddFontFromMemoryTTF(
                (void*)font_info->data,
                font_info->data_size,
                font_info->size,
                &config);
        } else if (font_info->data && font_info->data_size == 0) {
            // Load system font from file (filepath stored in data pointer)
            const char* filepath = (const char*)font_info->data;
            font_info->imgui_font = io.Fonts->AddFontFromFileTTF(
                filepath,
                font_info->size,
                &config);
        } else {
            // Use default font with custom size
            font_info->imgui_font = io.Fonts->AddFontDefault(&config);
        }

        if (font_info->imgui_font) {
            font_info->is_loaded = true;
            CORE_DEBUG("Loaded font: %s", font_info->name);
        } else {
            CORE_ERROR("Failed to load font: %s", font_info->name);
        }
    }

    // Build font atlas
    if (!io.Fonts->Build()) {
        CORE_ERROR("Failed to build font atlas");
        return false;
    }

    CORE_INFO("Successfully loaded %d fonts", font_registry.fonts.size());
    return true;
}

b8 ui_fonts_set_default(const char* name) {
    RUNTIME_ASSERT_MSG(name, "Font name cannot be null");

    UI_Font_Info* font_info = find_font_by_name(name);
    if (!font_info) {
        CORE_ERROR("Font '%s' not found", name);
        return false;
    }

    if (!font_info->is_loaded) {
        CORE_ERROR("Font '%s' not loaded", name);
        return false;
    }

    // Clear previous default
    for (u32 i = 0; i < font_registry.fonts.size(); ++i) {
        font_registry.fonts[i].is_default = false;
    }

    // Set new default
    font_info->is_default = true;
    font_registry.default_font = font_info->imgui_font;

    // Set ImGui default font
    ImGuiIO& io = ImGui::GetIO();
    io.FontDefault = font_registry.default_font;

    CORE_DEBUG("Set default font to: %s", name);
    return true;
}

ImFont* ui_fonts_find_by_style(const char* family, UI_Font_Weight weight, UI_Font_Style style) {
    RUNTIME_ASSERT_MSG(family, "Font family cannot be null");

    for (u32 i = 0; i < font_registry.fonts.size(); ++i) {
        const UI_Font_Info* font_info = &font_registry.fonts[i];

        if (font_info->is_loaded &&
            strcmp(font_info->family, family) == 0 &&
            font_info->weight == weight &&
            font_info->style == style) {
            return font_info->imgui_font;
        }
    }

    return nullptr;
}

b8 ui_fonts_rebuild() {
    if (!font_registry.is_initialized) {
        return false;
    }

    CORE_DEBUG("Rebuilding fonts...");

    // Mark all fonts as not loaded
    for (u32 i = 0; i < font_registry.fonts.size(); ++i) {
        font_registry.fonts[i].is_loaded = false;
        font_registry.fonts[i].imgui_font = nullptr;
    }

    // Clear font atlas
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    // Reload all fonts
    return ui_fonts_load_all();
}

b8 ui_fonts_load_system_defaults() {
    CORE_DEBUG("Loading system default fonts...");

    // Register common system fonts
    // Note: This is platform-specific and would need proper implementation
    // For now, we'll just register placeholders

    b8 success = true;

    // Add default font at various sizes using constexpr values
    success &= ui_fonts_register_embedded(
        "default_small", "Default", UI_Font_Weight::REGULAR, UI_Font_Style::NORMAL,
        default_font_data, default_font_data_size, UI_FONT_SIZE_SMALL);
    success &= ui_fonts_register_embedded(
        "default_normal", "Default", UI_Font_Weight::REGULAR, UI_Font_Style::NORMAL,
        default_font_data, default_font_data_size, UI_FONT_SIZE_NORMAL);
    success &= ui_fonts_register_embedded(
        "default_large", "Default", UI_Font_Weight::REGULAR, UI_Font_Style::NORMAL,
        default_font_data, default_font_data_size, UI_FONT_SIZE_LARGE);

    if (success) {
        CORE_DEBUG("System default fonts loaded successfully");
    } else {
        CORE_ERROR("Failed to load some system default fonts");
    }

    return success;
}

b8 ui_fonts_register_defaults() {
    CORE_DEBUG("Registering default embedded fonts...");

    b8 success = true;

    // Load embedded Roboto fonts directly using simplified interface
    ImFont* roboto_regular = assets_load_font(
        "roboto_regular",
        UI_FONT_SIZE_NORMAL);

    if (roboto_regular) {
        // Register the loaded font in our registry for name-based lookup
        UI_Font_Info font_info = {};
        font_info.name = "roboto_regular";
        font_info.family = "Roboto";
        font_info.weight = UI_Font_Weight::REGULAR;
        font_info.style = UI_Font_Style::NORMAL;
        font_info.size = UI_FONT_SIZE_NORMAL;
        font_info.imgui_font = roboto_regular;
        font_info.is_loaded = true;
        font_info.is_default = false;

        font_registry.fonts.push_back(font_info);
    }

    ImFont* roboto_bold = assets_load_font(
        "roboto_bold",
        UI_FONT_SIZE_NORMAL);

    if (roboto_bold) {
        UI_Font_Info font_info = {};
        font_info.name = "roboto_bold";
        font_info.family = "Roboto";
        font_info.weight = UI_Font_Weight::BOLD;
        font_info.style = UI_Font_Style::NORMAL;
        font_info.size = UI_FONT_SIZE_NORMAL;
        font_info.imgui_font = roboto_bold;
        font_info.is_loaded = true;
        font_info.is_default = false;

        font_registry.fonts.push_back(font_info);
    }

    ImFont* roboto_italic = assets_load_font(
		"roboto_italic", 
		UI_FONT_SIZE_NORMAL);

    if (roboto_italic) {
        UI_Font_Info font_info = {};
        font_info.name = "roboto_italic";
        font_info.family = "Roboto";
        font_info.weight = UI_Font_Weight::REGULAR;
        font_info.style = UI_Font_Style::ITALIC;
        font_info.size = UI_FONT_SIZE_NORMAL;
        font_info.imgui_font = roboto_italic;
        font_info.is_loaded = true;
        font_info.is_default = false;

        font_registry.fonts.push_back(font_info);
    }

    if (success) {
        CORE_DEBUG("Default embedded fonts registered successfully");
    } else {
        CORE_WARN("Failed to register some embedded fonts, using ImGui default");
    }

    return success;
}

// Internal function implementations

INTERNAL_FUNC UI_Font_Info* find_font_by_name(const char* name) {
    if (!name)
        return nullptr;

    // Check if font system is initialized
    if (!font_registry.is_initialized) {
        return nullptr;
    }

    // Create a temporary font info for searching
    UI_Font_Info search_font = {};
    search_font.name = name;

    // Use Auto_Array's find method
    UI_Font_Info* found = font_registry.fonts.find(search_font);

    // Check if found (find returns end() if not found)
    return (found < font_registry.fonts.end()) ? found : nullptr;
}

INTERNAL_FUNC ImFontConfig create_font_config(const UI_Font_Info* font_info) {
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false; // We manage our own font data
    config.MergeMode = false;
    config.PixelSnapH = true;
    config.GlyphMaxAdvanceX = FLT_MAX;
    config.RasterizerMultiply = 1.0f;
    config.EllipsisChar = (ImWchar)-1;

    // Set font name for debugging
    if (font_info->name) {
        strncpy(config.Name, font_info->name, sizeof(config.Name) - 1);
        config.Name[sizeof(config.Name) - 1] = '\0';
    }

    return config;
}
