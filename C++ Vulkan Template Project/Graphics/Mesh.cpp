#include "Mesh.h"
#include "../VulkanContext.h"

//private:
void Engine::Mesh::copyBuffer(VulkanContext& context, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = context.commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(context.device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(context.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(context.graphicsQueue);
    vkFreeCommandBuffers(context.device, context.commandPool, 1, &commandBuffer);
}
void Engine::Mesh::createBuffer(VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	ASSERT(vkCreateBuffer(context.device, &bufferInfo, nullptr, &buffer) == VK_SUCCESS);

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(context.device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = context.findMemoryType(memRequirements.memoryTypeBits, properties);

	ASSERT(vkAllocateMemory(context.device, &allocInfo, nullptr, &bufferMemory) == VK_SUCCESS);

    vkBindBufferMemory(context.device, buffer, bufferMemory, 0);
}
void Engine::Mesh::createVertexBuffer(VulkanContext& context) {
    //Vertex vertex;
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        
    void* data;
    vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(context.device, stagingBufferMemory);

    createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);
    copyBuffer(context, stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}
void Engine::Mesh::createIndexBuffer(VulkanContext& context) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(context.device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(context.device, stagingBufferMemory);

    createBuffer(context, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);
    copyBuffer(context, stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(context.device, stagingBuffer, nullptr);
    vkFreeMemory(context.device, stagingBufferMemory, nullptr);
}

void Engine::Mesh::create(VulkanContext& context, std::vector<Vertex>&& pVertices, std::vector<uint16_t>&& pIndices)
{
	//LOG("Creating Mesh with move semantics");
    vertices = std::move(pVertices);
    indices = std::move(pIndices);
    indexCount = static_cast<uint32_t>(indices.size());
    createVertexBuffer(context);
    createIndexBuffer(context);
}
void Engine::Mesh::create(VulkanContext& context, const std::vector<Vertex>& pVertices, const std::vector<uint16_t>& pIndices)
{
    //move later on
    vertices = pVertices;
    indices = pIndices;

    indexCount = static_cast<uint32_t>(indices.size());

    createVertexBuffer(context);
    createIndexBuffer(context);
}
void Engine::Mesh::cleanup(VulkanContext& context)
{
    vkDestroyBuffer(context.device, indexBuffer, nullptr);
    vkFreeMemory(context.device, indexBufferMemory, nullptr);
    vkDestroyBuffer(context.device, vertexBuffer, nullptr);
    vkFreeMemory(context.device, vertexBufferMemory, nullptr);
}
void Engine::Mesh::bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer)
{
    if (instanceBuffer != VK_NULL_HANDLE) {
        // Bind vertex buffer (binding 0) and instance buffer (binding 1)
        VkBuffer vertexBuffers[] = { vertexBuffer, instanceBuffer };
        VkDeviceSize offsets[] = { 0, 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
    }
    else {
        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    }
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
}
void Engine::Mesh::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) {
    vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
}

