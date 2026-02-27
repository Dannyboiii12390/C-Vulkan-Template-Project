#include "ParticlePipeline.h"
#include "../../VulkanContext.h"
#include "../../Core/Debug Utils.h"

namespace Engine
{
    ParticlePipeline::~ParticlePipeline() = default;

    void ParticlePipeline::create(
        VulkanContext& context,
        const std::string& pVertShaderPath,
        const std::string& pFragShaderPath,
        VkFormat pColorFormat,
        VkFormat pDepthFormat,
        VkDescriptorSetLayout pDescriptorSetLayout,
        VkCullModeFlags cullMode, // NEW: desired cull mode
        bool depthWrite                            // NEW: depth write enable
    ) {
        const VkDevice device = context.getDevice();

        // Load shaders
        const auto vertShaderCode = readFile(pVertShaderPath);
        const auto fragShaderCode = readFile(pFragShaderPath);

        const VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
        const VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

        // Shader stage creation
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = getName();

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = getName();

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        // Particle vertex input - only particle position
        const auto attributeDescriptions = Engine::ParticleVertex::getAttributeDescriptions();
        const auto bindingDescription = Engine::ParticleVertex::getBindingDescription();
        const std::vector<VkVertexInputBindingDescription> bindingDescriptions = { bindingDescription };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = createVertexInputState(attributeDescriptions, bindingDescriptions);
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssemblyState();
        VkPipelineViewportStateCreateInfo viewportState = createViewportState();

        // Rasterizer: no culling for particles
        VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizationState();
        rasterizer.cullMode = VK_CULL_MODE_NONE;

        VkPipelineMultisampleStateCreateInfo multisampling = createMultisampleState();

        // Color blend attachment with additive blending for fire effect
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE; // Additive blending
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlendState(colorBlendAttachment);
        VkPipelineDynamicStateCreateInfo dynamicState = createDynamicState();

        // Store and validate the incoming descriptor set layout (fixes VK_NULL_HANDLE use)
        setDescriptorSetLayout(pDescriptorSetLayout);
        VkDescriptorSetLayout descriptorSetLayout = getDescriptorSetLayout();
        ASSERT(descriptorSetLayout != VK_NULL_HANDLE);

        // Add a push-constant range so each ParticleSystem can push a model matrix
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(Engine::PushConstantModel);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        VkPipelineLayout createdLayout = VK_NULL_HANDLE;
        ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &createdLayout) == VK_SUCCESS);
        setLayout(createdLayout);

        // Dynamic rendering info
        VkPipelineRenderingCreateInfo renderingCreateInfo{};
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.pColorAttachmentFormats = &pColorFormat;
        renderingCreateInfo.depthAttachmentFormat = pDepthFormat;
        renderingCreateInfo.stencilAttachmentFormat =
            (pDepthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || pDepthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
            ? pDepthFormat
            : VK_FORMAT_UNDEFINED;

        // Depth stencil state: depth test enabled, depth write disabled
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_FALSE; // Don't write to depth buffer
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        // Create the graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = &renderingCreateInfo;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = createdLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;
        pipelineInfo.pDepthStencilState = &depthStencil;

        VkPipeline createdPipeline = VK_NULL_HANDLE;
        ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &createdPipeline) == VK_SUCCESS);
        setPipeline(createdPipeline);

        // Cleanup shader modules
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
    }

    void ParticlePipeline::destroy(VkDevice device)
    {
        Pipeline::destroy(device);
    }

    VkPipelineInputAssemblyStateCreateInfo ParticlePipeline::createInputAssemblyState()
    {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;  // Use points instead of triangles
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        return inputAssembly;
    }

    VkPipelineRasterizationStateCreateInfo ParticlePipeline::createRasterizationState()
    {
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;  // Don't cull points
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        return rasterizer;
    }
}