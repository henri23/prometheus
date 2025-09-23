#include "vulkan_object_shader.hpp"
#include "renderer/vulkan/vulkan_types.hpp"

#include "core/logger.hpp"
#include "renderer/vulkan/vulkan_shader_utils.hpp"

#define BUILTIN_SHADER_NAME_OBJECT "Builtin.ObjectShader"

b8 vulkan_object_shader_create(
    Vulkan_Context* context,
    Vulkan_Object_Shader* out_shader) {

    char stage_type_strs[OBJECT_SHADER_STAGE_COUNT][5] = {"vert", "frag"};

    VkShaderStageFlagBits state_types[OBJECT_SHADER_STAGE_COUNT] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT};

    for (u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
        if (!create_shader_module(
                context,
                BUILTIN_SHADER_NAME_OBJECT,
                stage_type_strs[i],
                state_types[i],
                i,
                out_shader->stages)) {

            CORE_ERROR(
                "Failed to create %s shader module for '%s'",
                stage_type_strs[i],
                BUILTIN_SHADER_NAME_OBJECT);

            return false;
        }
    }

    return true;
}

void vulkan_object_shader_destroy(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {

	for(u32 i = 0; i < OBJECT_SHADER_STAGE_COUNT; ++i) {
		vkDestroyShaderModule(
			context->device.logical_device, 
			shader->stages[i].handle, 
			context->allocator);
	}
}

void vulkan_object_shader_use(
    Vulkan_Context* context,
    Vulkan_Object_Shader* shader) {
    (void)context;
    (void)shader;
}
