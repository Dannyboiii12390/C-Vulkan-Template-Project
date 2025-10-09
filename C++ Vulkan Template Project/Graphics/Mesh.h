#pragma once
#include <vector>
#include "Vertex.h"
#include <vulkan/vulkan.h>

//struct Vertex;
class VulkanContext;

namespace Engine 
{
	class Mesh
	{
		//VulkanContext& context;

		// CPU-side data
		std::vector<Vertex> vertices;
		std::vector<uint16_t> indices;

		// GPU-side resources
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

		uint32_t indexCount;

		void createVertexBuffer(VulkanContext& context);
		void createIndexBuffer(VulkanContext& context);
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VulkanContext& context);
		void createBuffer(VulkanContext& context, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	public:
		//Mesh(VulkanContext& context, const std::vector<Vertex>& vertices, const std::vector<uint16_t>& indices) : vertices(vertices), indices(indices), indexCount(static_cast<uint32_t>(indices.size()));
		Mesh() = default;
		void create(VulkanContext& context, const std::vector<Vertex>& pVertices, const std::vector<uint16_t>& pIndices);
		void cleanup(VulkanContext& context);
		void bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer = VK_NULL_HANDLE);
		void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount = 1);

	};
}


