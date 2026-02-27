#include "Mesh.h"
#include "../VulkanContext.h"
#include <array>
namespace Engine
{
    void Mesh::create(const VulkanContext& context, std::vector<Vertex>&& pVertices, std::vector<uint16_t>&& pIndices)
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
    void Mesh::create(const VulkanContext& context, std::vector<Vertex>&& pVertices)
    {
        //LOG("Creating Mesh with move semantics - non indexed");
        vertices = std::move(pVertices);
        indexCount = 0;
        vertexBuffer = Buffer::createVertexBuffer(context, vertices.data(), sizeof(vertices[0]) * vertices.size());
    }
    void Mesh::cleanup(const VulkanContext& context)
    {
        if(isIndexed()) indexBuffer.destroy(context.getDevice());
        vertexBuffer.destroy(context.getDevice());
    }
    void Mesh::bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer) const 
    {
        if (instanceBuffer != VK_NULL_HANDLE) {
            // Bind vertex buffer (binding 0) and instance buffer (binding 1)
            std::array<VkBuffer, 2> vertexBuffers = { vertexBuffer.getBuffer(), instanceBuffer };
            std::array<VkDeviceSize, 2> offsets = { 0, 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers.data(), offsets.data());
        }
        else {
            std::array<VkBuffer, 2> vertexBuffers = { vertexBuffer.getBuffer(), VK_NULL_HANDLE };
            std::array<VkDeviceSize, 2> offsets = { 0, 0 };
            // bind only the first buffer when no instance buffer is provided
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers.data(), offsets.data());
        }
        if (isIndexed()) vkCmdBindIndexBuffer(commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT16);
    }
    void Mesh::draw(VkCommandBuffer commandBuffer, uint32_t instanceCount) const
    {
        if (isIndexed()) vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, 0, 0, 0);
        else vkCmdDraw(commandBuffer, vertices.size(), instanceCount, 0, 0);
    }
    Mesh Mesh::copy(const VulkanContext& context, const Mesh& other)
    {
        Mesh newMesh;
        // Create GPU resources by copying CPU-side data and forwarding prvalue vectors
        if (!other.isIndexed()) {
            // construct a temporary vector<Vertex> from other.vertices and forward it
            newMesh.create(context, std::vector<Vertex>(other.vertices));
        }
        else {
            // construct temporaries for vertices and indices and forward them
            newMesh.create(context,
                std::vector<Vertex>(other.vertices),
                std::vector<uint16_t>(other.indices));
        }
        return newMesh;
	}
}
