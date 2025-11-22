#include "SkyboxPipeline.h"
#include "../../VulkanContext.h"
#include <stdexcept>

namespace Engine
{
	void SkyboxPipeline::create(
		VulkanContext& context,
		const std::string& vertShaderPath,
		const std::string& fragShaderPath,
		VkFormat colorFormat,
		VkFormat depthFormat,
		VkDescriptorSetLayout descriptorSetLayout)
	{
		VkDevice device = context.getDevice();

		// Load shaders
		auto vertShaderCode = readFile(vertShaderPath);
		auto fragShaderCode = readFile(fragShaderPath);

		VkShaderModule vertShaderModule = createShaderModule(device, vertShaderCode);
		VkShaderModule fragShaderModule = createShaderModule(device, fragShaderCode);

		// Shader stage creation
		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// Use the same vertex binding/attribute descriptions if compatible.
		// Skybox shader uses only position; reusing Engine::Vertex descriptors is OK if mesh matches.
		auto attributeDescriptions = Engine::Vertex::getAttributeDescriptions();
		auto bindingDescriptions = Engine::Vertex::getBindingDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = createVertexInputState(attributeDescriptions, bindingDescriptions);
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = createInputAssemblyState();
		VkPipelineViewportStateCreateInfo viewportState = createViewportState();

		// Rasterizer: cull front faces because we're inside the cube
		VkPipelineRasterizationStateCreateInfo rasterizer = createRasterizationState();
		rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;

		VkPipelineMultisampleStateCreateInfo multisampling = createMultisampleState();

		// Color blend attachment
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = createColorBlendState(colorBlendAttachment);
		VkPipelineDynamicStateCreateInfo dynamicState = createDynamicState();

		// Push constants
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(Engine::PushConstantModel);

		// Create pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

		// Dynamic rendering info
		VkPipelineRenderingCreateInfo renderingCreateInfo{};
		renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		renderingCreateInfo.colorAttachmentCount = 1;
		renderingCreateInfo.pColorAttachmentFormats = &colorFormat;
		renderingCreateInfo.depthAttachmentFormat = depthFormat;
		// set stencil format only when depthFormat has a stencil component
		renderingCreateInfo.stencilAttachmentFormat =
			(depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
			? depthFormat
			: VK_FORMAT_UNDEFINED;

		// Depth stencil state: depth test enabled, but depth write disabled so skybox sits at far plane
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.front = {}; depthStencil.back = {};

		// Create the graphics pipeline (for skybox)
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
		pipelineInfo.layout = pipelineLayout;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
		pipelineInfo.subpass = 0;
		pipelineInfo.pDepthStencilState = &depthStencil;

		ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) == VK_SUCCESS);

		// Cleanup shader modules
		vkDestroyShaderModule(device, fragShaderModule, nullptr);
		vkDestroyShaderModule(device, vertShaderModule, nullptr);
	}

	void SkyboxPipeline::destroy(VkDevice device) {
		Pipeline::destroy(device);
	}
}