#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

class VulkanContext;
namespace Engine 
{
	class Buffer final
	{
	public:

		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		VkMemoryPropertyFlags properties = 0;

		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&&) noexcept = default;
		Buffer& operator=(Buffer&&) noexcept = default;
		~Buffer() = default;

		void create(const VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

		void destroy(VkDevice device);
		void bind(VkDevice device, VkDeviceSize offset = 0);
		void* map(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize mapSize = VK_WHOLE_SIZE);
		void unmap(VkDevice device);
		void write(VkDevice device, const void* srcData, VkDeviceSize writeSize, VkDeviceSize offset = 0);

		void flush(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize flushSize = VK_WHOLE_SIZE);
		void invalidate(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize invalidateSize = VK_WHOLE_SIZE);
		static void copy(class VulkanContext& ctx, const Buffer& src, Buffer& dst, VkDeviceSize copySize);

		bool isHostVisible() const noexcept { return (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }

		static Buffer createVertexBuffer(class VulkanContext& ctx, const void* data, VkDeviceSize size);
		static Buffer createIndexBuffer(class VulkanContext& ctx, const void* data, VkDeviceSize size);
		static Buffer createUniformBuffer(class VulkanContext& ctx, VkDeviceSize size);

		uint32_t findMemoryType(const VulkanContext& context, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	};
}
