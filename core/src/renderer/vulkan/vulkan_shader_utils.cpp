#include "vulkan_shader_utils.hpp"

#include "core/logger.hpp"
#include "memory/memory.hpp"

#include "platform/filesystem.hpp"
#include <cstdio>

b8 create_shader_module(
    Vulkan_Context* context,
    const char* name,
    const char* type_str,
    VkShaderStageFlagBits shader_stage_flag,
    u32 stage_index,
    Vulkan_Shader_Stage* shader_stages) {

    char file_name[512];
    // TODO: Change to not hardcoded path
    sprintf(file_name, "assets/shaders/%s.%s.spv", name, type_str);

    memory_zero(
        &shader_stages[stage_index].create_info,
        sizeof(VkShaderModuleCreateInfo));

    shader_stages[stage_index].create_info.sType =
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

    File_Handle handle;
    if (!filesystem_open(
            file_name,
            File_Modes::READ,
            true,
            &handle)) {
        CORE_ERROR("Unable to read shader module: %s", file_name);
        return false;
    }

    // Read the entire file as binary
    u64 size = 0;
    u8* file_byte_array = nullptr;

    if (!filesystem_read_all_bytes(
            &handle,
            &file_byte_array,
            &size)) {
        CORE_ERROR("Unable to read bytes of the binary file: %s", file_name);
        return false;
    }

    // Set the size in bytes of the byte code
    shader_stages[stage_index].create_info.codeSize = size;
    shader_stages[stage_index].create_info.pCode = reinterpret_cast<u32*>(
        file_byte_array);

    filesystem_close(&handle);

    VK_CHECK(
        vkCreateShaderModule(
            context->device.logical_device,
            &shader_stages[stage_index].create_info,
            context->allocator,
            &shader_stages[stage_index].handle));

    memory_zero(
        &shader_stages[stage_index].shader_stage_create_info,
        sizeof(VkPipelineShaderStageCreateInfo));

    shader_stages[stage_index].shader_stage_create_info.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

    shader_stages[stage_index].shader_stage_create_info.stage =
        shader_stage_flag;

    shader_stages[stage_index].shader_stage_create_info.module =
        shader_stages[stage_index].handle;

    shader_stages[stage_index].shader_stage_create_info.pName =
        "main";

    if (file_byte_array) {
        memory_deallocate(file_byte_array, size, Memory_Tag::STRING);
        file_byte_array = nullptr;
    }

    return true;
}
