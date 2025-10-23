#include "Mesh.h"
#include "../VulkanContext.h"
namespace Engine
{
    void Mesh::create(VulkanContext& context, std::vector<Vertex>&& pVertices, std::vector<uint16_t>&& pIndices)
    {
        //LOG("Creating Mesh with move semantics");
        vertices = std::move(pVertices);
        indices = std::move(pIndices);
        indexCount = static_cast<uint32_t>(indices.size());
        //createVertexBuffer(context);
        //createIndexBuffer(context);
        vertexBuffer = Buffer::createVertexBuffer(context, vertices.data(), sizeof(vertices[0]) * vertices.size());
        indexBuffer = Buffer::createIndexBuffer(context, indices.data(), sizeof(indices[0]) * indices.size());
    }
    void Mesh::create(VulkanContext& context, const std::vector<Vertex>& pVertices, const std::vector<uint16_t>& pIndices)
    {
        //move later on
        vertices = pVertices;
        indices = pIndices;
        indexCount = static_cast<uint32_t>(indices.size());
        //createVertexBuffer(context);
        //createIndexBuffer(context);
        vertexBuffer = Buffer::createVertexBuffer(context, vertices.data(), sizeof(vertices[0]) * vertices.size());
        if(isIndexed()) indexBuffer = Buffer::createIndexBuffer(context, indices.data(), sizeof(indices[0]) * indices.size());
    }
    void Mesh::cleanup(VulkanContext& context)
    {
        if(isIndexed()) indexBuffer.destroy(context.getDevice());
        vertexBuffer.destroy(context.getDevice());
    }
    void Mesh::bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer)
    {
        if (instanceBuffer != VK_NULL_HANDLE) {
            // Bind vertex buffer (binding 0) and instance buffer (binding 1)
            VkBuffer vertexBuffers[] = { vertexBuffer.buffer, instanceBuffer };
            VkDeviceSize offsets[] = { 0, 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
        }
        else {
            VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        }
        if (isIndexed()) vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);
    }
    void Mesh::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) {
        if(isIndexed()) vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        else vkCmdDraw(commandBuffer, vertices.size(), 1, 0, 0);


    }
}
