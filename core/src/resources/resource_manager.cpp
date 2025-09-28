#include "resource_manager.hpp"
#include "loaders/binary_loader.hpp"
#include "loaders/image_loader.hpp"
#include "renderer/renderer_frontend.hpp"
#include "core/logger.hpp"
#include "stb_image.h"

b8 resource_manager_initialize() {
    CORE_DEBUG("Initializing resource manager...");

    if (!binary_loader_initialize()) {
        CORE_ERROR("Failed to initialize binary loader");
        return false;
    }

    if (!image_loader_initialize()) {
        CORE_ERROR("Failed to initialize image loader");
        binary_loader_shutdown();
        return false;
    }

    CORE_DEBUG("Resource manager initialized successfully");
    return true;
}

void resource_manager_shutdown() {
    CORE_DEBUG("Shutting down resource manager...");

    image_loader_shutdown();
    binary_loader_shutdown();

    CORE_DEBUG("Resource manager shut down");
}

const u8* resource_get_binary_data(const char* resource_name, u64* out_size) {
    return binary_loader_get_data(resource_name, out_size);
}

b8 resource_load_image(const char* image_name, UI_Image_Resource** out_image_resource) {
    if (!image_name) {
        CORE_ERROR("Image name cannot be null");
        return false;
    }

    if (!out_image_resource) {
        CORE_ERROR("Output image resource pointer cannot be null");
        return false;
    }

    // Load and decode image data
    Image_Load_Result result = image_loader_load(image_name);
    if (!result.success) {
        CORE_ERROR("Failed to load image '%s': %s", image_name, result.error_message);
        return false;
    }

    // Allocate UI image resource (caller manages this memory)
    UI_Image_Resource* ui_resource = new UI_Image_Resource();

    // Create GPU resource through renderer
    b8 success = renderer_create_ui_image(
        result.width,
        result.height,
        result.pixel_data,
        result.pixel_data_size,
        ui_resource);

    // Clean up decoded pixel data (renderer has copied it)
    stbi_image_free(result.pixel_data);

    if (!success) {
        CORE_ERROR("Failed to create GPU image resource for '%s'", image_name);
        delete ui_resource;
        return false;
    }

    // Set the output parameter
    *out_image_resource = ui_resource;

    CORE_DEBUG("Successfully loaded image resource: %s (%ux%u)", image_name, result.width, result.height);
    return true;
}

void resource_free_image(UI_Image_Resource* resource) {
    if (!resource) {
        CORE_WARN("Attempted to free null image resource");
        return;
    }

    renderer_destroy_ui_image(resource);
    delete resource; // Free the UI_Image_Resource memory
    CORE_DEBUG("Freed image resource");
}