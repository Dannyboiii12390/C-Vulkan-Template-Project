#include "Shader.h"
#include "../VulkanContext.h"
#include "VulkanTypes.h"
#include <fstream>

namespace Engine
{
    void Shader::create(
        VulkanContext& context,
        const std::string& vertShaderPath,
        const std::string& fragShaderPath,
        const std::vector<ShaderDescriptorBinding>& bindingInfos,
        VkFormat colorFormat,
        VkFormat depthFormat,
        uint32_t swapchainImageCount,
        VkPushConstantRange* pushConstantRange)
    {
        this->vertShaderPath = vertShaderPath;
        this->fragShaderPath = fragShaderPath;
        this->bindings = bindingInfos;

        VkDevice device = context.getDevice();

        // Load shader modules
        auto vertShaderCode = readFile(vertShaderPath);
        auto fragShaderCode = readFile(fragShaderPath);

        vertShaderModule = createShaderModule(device, vertShaderCode);
        fragShaderModule = createShaderModule(device, fragShaderCode);

        // Create descriptor set layout
        std::vector<VkDescriptorSetLayoutBinding> layoutBindings;
        layoutBindings.reserve(bindingInfos.size());

        for (const auto& binding : bindingInfos)
        {
            VkDescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.binding = binding.binding;
            layoutBinding.descriptorCount = binding.descriptorCount;
            layoutBinding.descriptorType = binding.descriptorType;
            layoutBinding.stageFlags = binding.stageFlags;
            layoutBinding.pImmutableSamplers = nullptr;
            layoutBindings.push_back(layoutBinding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
        layoutInfo.pBindings = layoutBindings.data();

        ASSERT(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS);

        // Create pipeline
        createPipeline(context, colorFormat, depthFormat, pushConstantRange);
    }

    void Shader::createPipeline(
        VulkanContext& context,
        VkFormat colorFormat,
        VkFormat depthFormat,
        VkPushConstantRange* pushConstantRange)
    {
        VkDevice device = context.getDevice();

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

        // Get vertex binding and attribute descriptions
        auto attributeDescriptions = Vertex::getAttributeDescriptions();
        auto bindingDescription = Vertex::getBindingDescription();
        const std::vector<VkVertexInputBindingDescription> bindingDescriptionsVec = { bindingDescription };

        // Vertex input state
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptionsVec.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptionsVec.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        // Input assembly state
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // Viewport state
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;

        // Rasterization state
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_NONE;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_TRUE;

        // Multisample state
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Color blend attachment
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        // Color blend state
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;

        // Dynamic state
        std::vector<VkDynamicState> dynamicStates = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        // Pipeline layout creation
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

        if (pushConstantRange != nullptr)
        {
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = pushConstantRange;
        }

        ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) == VK_SUCCESS);

        // Depth stencil state
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        // Dynamic rendering info
        VkPipelineRenderingCreateInfo renderingCreateInfo{};
        renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        renderingCreateInfo.colorAttachmentCount = 1;
        renderingCreateInfo.pColorAttachmentFormats = &colorFormat;
        renderingCreateInfo.depthAttachmentFormat = depthFormat;
        renderingCreateInfo.stencilAttachmentFormat =
            (depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT || depthFormat == VK_FORMAT_D24_UNORM_S8_UINT)
            ? depthFormat
            : VK_FORMAT_UNDEFINED;

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
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = VK_NULL_HANDLE;
        pipelineInfo.subpass = 0;

        ASSERT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline) == VK_SUCCESS);
    }

    void Shader::createDescriptorSets(
        VkDevice device,
        VkDescriptorPool descriptorPool,
        uint32_t swapchainImageCount)
    {
        ASSERT(descriptorSetLayout != VK_NULL_HANDLE);

        // Allocate descriptor sets
        std::vector<VkDescriptorSetLayout> layouts(swapchainImageCount, descriptorSetLayout);

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = swapchainImageCount;
        allocInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(swapchainImageCount);
        ASSERT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) == VK_SUCCESS);
    }

    void Shader::updateDescriptorSet(
        VkDevice device,
        uint32_t frameIndex,
        const std::vector<VkDescriptorBufferInfo>& bufferInfos,
        const std::vector<VkDescriptorImageInfo>& imageInfos)
    {
        ASSERT(frameIndex < descriptorSets.size());

        std::vector<VkWriteDescriptorSet> descriptorWrites;

        // Track which binding index we're processing
        size_t bufferIndex = 0;
        size_t imageIndex = 0;

        for (const auto& binding : bindings)
        {
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = descriptorSets[frameIndex];
            write.dstBinding = binding.binding;
            write.dstArrayElement = 0;
            write.descriptorType = binding.descriptorType;
            write.descriptorCount = binding.descriptorCount;

            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
                binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
            {
                ASSERT(bufferIndex < bufferInfos.size());
                write.pBufferInfo = &bufferInfos[bufferIndex];
                bufferIndex++;
            }
            else if (binding.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                binding.descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
            {
                ASSERT(imageIndex < imageInfos.size());
                write.pImageInfo = &imageInfos[imageIndex];
                imageIndex += binding.descriptorCount;
            }

            descriptorWrites.push_back(write);
        }

        if (!descriptorWrites.empty())
        {
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
                descriptorWrites.data(), 0, nullptr);
        }
    }

    void Shader::bind(VkCommandBuffer commandBuffer, uint32_t frameIndex) const
    {
        ASSERT(pipeline != VK_NULL_HANDLE);
        ASSERT(frameIndex < descriptorSets.size());

        // Bind pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        // Bind descriptor set
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &descriptorSets[frameIndex], 0, nullptr);
    }

    void Shader::destroy(VkDevice device)
    {
        // Destroy pipeline
        if (pipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(device, pipeline, nullptr);
            pipeline = VK_NULL_HANDLE;
        }

        // Destroy pipeline layout
        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            pipelineLayout = VK_NULL_HANDLE;
        }

        // Destroy shader modules
        if (vertShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
            vertShaderModule = VK_NULL_HANDLE;
        }

        if (fragShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            fragShaderModule = VK_NULL_HANDLE;
        }

        // Note: Descriptor sets are automatically freed when the pool is destroyed
        descriptorSets.clear();

        if (descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
            descriptorSetLayout = VK_NULL_HANDLE;
        }

        bindings.clear();
    }

    std::vector<char> Shader::readFile(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        ASSERT(file.is_open());

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();

        return buffer;
    }

    VkShaderModule Shader::createShaderModule(VkDevice device, const std::vector<char>& code)
    {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        VkShaderModule shaderModule;
        ASSERT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS);

        return shaderModule;
    }

} // namespace Engine