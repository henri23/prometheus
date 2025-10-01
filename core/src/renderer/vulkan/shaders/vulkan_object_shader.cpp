#include "vulkan_object_shader.hpp"
#include "renderer/vulkan/vulkan_pipeline.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

#include "core/logger.hpp"
#include "renderer/vulkan/vulkan_shader_utils.hpp"

#include "math/math_types.hpp"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vulkan_object_shader_create(Vulkan_Context* context,
    Vulkan_Object_Shader* out_shader) {

    char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};

    VkShaderStageFlagBits state_types[OBJECT_SHADER_STAGE_COUNT] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(context,
                BUILTIN_SHADER_NAME_OBJECT,
                stage_type_strs[i],
                state_types[i],
                i,
                out_shader->stages)) {

            CORE_ERROR("Failed to create %s shader module for '%s'",
                stage_type_strs[i],
                BUILTIN_SHADER_NAME_OBJECT);

            return false;
        }
    }

    // TODO: add descriptors

    // Pipeline creation
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = (f32)context->main_target.height;
    viewport.width = (f32)context->main_target.width;
    viewport.height = -(f32)context->main_target.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width = context->main_target.width;
    scissor.extent.height = context->main_target.height;

    u32 offset = 0;
    constexpr s32 attribute_count = 1;

    VkVertexInputAttributeDescription attribute_descriptions[attribute_count];

    // Position
    VkFormat formats[attribute_count] = {VK_FORMAT_R32G32B32_SFLOAT};
    u64 sizes[attribute_count] = {sizeof(vec3)};

    for (u32 i = 0; i < attribute_count; ++i) {
        attribute_descriptions[i].binding = 0;
        attribute_descriptions[i].location = i;
        attribute_descriptions[i].format = formats[i];
        attribute_descriptions[i].offset = offset;
        offset += sizes[i];
    }

    VkPipelineShaderStageCreateInfo
        stage_create_infos[OBJECT_SHADER_STAGE_COUNT];
    memory_zero(stage_create_infos, sizeof(stage_create_infos));
    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        stage_create_infos[i].sType =
            out_shader->stages[i].shader_stage_create_info.sType;

        stage_create_infos[i] = out_shader->stages[i].shader_stage_create_info;
    }

    if (!vulkan_graphics_pipeline_create(context,
            &context->main_renderpass,
            attribute_count,
            attribute_descriptions,
            0,
            0,
            OBJECT_SHADER_STAGE_COUNT,
            stage_create_infos,
            viewport,
            scissor,
            false,
            &out_shader->pipeline)) {

        CORE_ERROR("Failed to load graphics pipeline for object shader");
        return false;
    }

    return true;
}

void vulkan_object_shader_destroy(Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {

    vulkan_graphics_pipeline_destroy(context, &shader->pipeline);

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        vkDestroyShaderModule(context->device.logical_device,
            shader->stages[i].handle,
            context->allocator);
    }
}

void vulkan_object_shader_use(Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {
    u32 image_index = context->current_frame;
    vulkan_graphics_pipeline_bind(&context->main_command_buffers[image_index],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        &shader->pipeline);
}
