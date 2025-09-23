#include "vulkan_pipeline.hpp"
#include "vulkan_utils.hpp"

#include "core/logger.hpp"
#include "memory/memory.hpp"

b8 vulkan_graphics_pipeline_create(
    Vulkan_Context* context,
    Vulkan_Renderpass* renderpass,
    u32 attribute_count,
    VkVertexInputAttributeDescription* attributes,
    u32 descriptor_set_layout_count,
    VkDescriptorSetLayout* scriptor_set_layouts,
    u32 stage_count,
    VkPipelineShaderStageCreateInfo* stages,
    VkViewport viewport,
    VkRect2D scissor,
    b8 is_wireframe,
    Vulkan_Pipeline* out_pipeline) {

    // Viewport state creation
    VkPipelineViewportStateCreateInfo viewport_state = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};

    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    // If depthClampEnable is set to VK_TRUE, the fragments beyond the near/far
    // planes are clamped rather than discarted
    rasterizer_create_info.depthClampEnable = VK_FALSE;

    // If rasterizerDiscardEnable is set to VK_TRUE, geometry never reaches the
    // rasterizer stage (useful for transform feedback)
    rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;

    // Specifies whether to fill the polygons or render as wireframe
    rasterizer_create_info.polygonMode =
        is_wireframe
            ? VK_POLYGON_MODE_LINE
            : VK_POLYGON_MODE_FILL;

    // If backface culling is enabled, triangles facing away from the camera are
    // discarded
    rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;

    // Desines what is considered the front face of a triangle
    rasterizer_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    // Disables depth bias. Depth bias is used to tweak depth values to avoid
    // z-fighting. typically for decals or shadow mapping
    rasterizer_create_info.depthBiasEnable = VK_FALSE;

    // Since no bias is applied, I leave these parameters zerod out
    rasterizer_create_info.depthBiasConstantFactor = 0.0f;
    rasterizer_create_info.depthBiasClamp = 0.0f;
    rasterizer_create_info.depthBiasSlopeFactor = 0.0f;

    // Multisampling - Not used for now so set to defaults
    VkPipelineMultisampleStateCreateInfo multisampling_create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    multisampling_create_info.sampleShadingEnable = VK_FALSE;
    multisampling_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling_create_info.minSampleShading = 1.0f;
    multisampling_create_info.pSampleMask = 0;
    multisampling_create_info.alphaToCoverageEnable = VK_FALSE;
    multisampling_create_info.alphaToOneEnable = VK_FALSE;

    // Depth and stencil testing
    VkPipelineDepthStencilStateCreateInfo depth_stencil = {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
    color_blend_attachment_state.blendEnable = VK_TRUE;
    color_blend_attachment_state.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;

    color_blend_attachment_state.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment_state.srcAlphaBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;

    color_blend_attachment_state.dstAlphaBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT |
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;

    return true;
}

void vulkan_graphics_pipeline_destroy(
    Vulkan_Context* context,
    Vulkan_Pipeline* pipeline) {
}

void vulkan_graphics_pipeline_bind(
    Vulkan_Command_Buffer* command_buffer,
    VkPipelineBindPoint bind_point,
    Vulkan_Pipeline* pipeline) {
}
