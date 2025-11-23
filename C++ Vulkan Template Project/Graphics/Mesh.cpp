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
    void Mesh::create(VulkanContext& context, std::vector<Vertex>&& pVertices)
    {
        //LOG("Creating Mesh with move semantics - non indexed");
        vertices = std::move(pVertices);
        indexCount = 0;
        vertexBuffer = Buffer::createVertexBuffer(context, vertices.data(), sizeof(vertices[0]) * vertices.size());
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
        else vkCmdDraw(commandBuffer, vertices.size(), instanceCount, 0, 0);


    }

    /*
    Pseudocode / Plan (detailed):
    - Create a new Mesh object 'newMesh'.
    - Determine if 'other' is indexed by calling 'other.isIndexed()' (preferred over checking indexCount).
    - If not indexed:
        - Make a copy of 'other.vertices' into a temporary std::vector<Vertex> (prvalue).
        - Call 'newMesh.create(context, <temp_vertices>)' which takes an rvalue reference.
    - If indexed:
        - Make copies of 'other.vertices' and 'other.indices' into temporary prvalue vectors
          of matching element types (Vertex and uint16_t).
        - Call 'newMesh.create(context, <temp_vertices>, <temp_indices>)'.
    - Return the newly created mesh.
    Notes:
    - Use direct prvalue construction of std::vector to bind to rvalue references; do NOT use
      non-existent std::make_move. Avoid incorrect element-type conversions (e.g., uint32_t).
    */

    Mesh Mesh::copy(VulkanContext& context, const Mesh& other)
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
