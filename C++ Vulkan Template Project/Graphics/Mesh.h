#pragma once
#include <vector>
#include "VulkanTypes.h"
#include <vulkan/vulkan.h>
#include "Buffer.h"

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
		Buffer vertexBuffer;
		Buffer indexBuffer;

		uint32_t indexCount = 0;

	public:

		Mesh() = default;
		void create(VulkanContext& context, std::vector<Vertex>&& pVertices, std::vector<uint16_t>&& pIndices);
		void create(VulkanContext& context, const std::vector<Vertex>& pVertices, const std::vector<uint16_t>& pIndices);
		void cleanup(VulkanContext& context);
		void bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer = VK_NULL_HANDLE);
		void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount = 1);
		bool isIndexed() { return indices.size() != 0; }

	};
}


