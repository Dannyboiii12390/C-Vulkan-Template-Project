#pragma once
#include <vector>
#include "VulkanTypes.h"
#include <vulkan/vulkan.h>
#include "Buffer.h"

//struct Vertex;
class VulkanContext;

namespace Engine 
{
	class Mesh final
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

		// Deep-copy: copy CPU-side data; GPU resources are reset (cannot copy without VulkanContext)
		// instead i made a static copy method, that makes a copy of index and vertex data, then creates a new mesh with new GPU resources
		//Mesh(const Mesh& other) = delete; // disable copy constructor
		//Mesh& operator=(const Mesh& other) = delete; // disable copy assignment
		static Mesh copy(VulkanContext& context, const Mesh& other);

		void create(VulkanContext& context, std::vector<Vertex>&& pVertices, std::vector<uint16_t>&& pIndices);
		void create(VulkanContext& context, std::vector<Vertex>&& pVertices);
		void cleanup(VulkanContext& context);
		void bind(VkCommandBuffer commandBuffer, VkBuffer instanceBuffer = VK_NULL_HANDLE);
		void draw(VkCommandBuffer commandBuffer, uint32_t instanceCount = 1);
		bool isIndexed() const { return indices.size() != 0; }

		bool operator==(const Mesh& other) const
		{
			return (vertices.size() == other.vertices.size()) && (indices.size() == other.indices.size());
		}


	};
}


