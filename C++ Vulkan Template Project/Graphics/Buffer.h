#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

class VulkanContext;
namespace Engine 
{
	class Buffer final
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;

		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		VkMemoryPropertyFlags properties = 0;
	public:

		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer& operator=(const Buffer&) = delete;
		Buffer(Buffer&&) noexcept = default;
		Buffer& operator=(Buffer&&) noexcept = default;
		~Buffer() = default;

		inline VkBuffer getBuffer() const noexcept { return buffer; }

		void create(const VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

		void destroy(VkDevice device);
		void bind(VkDevice device, VkDeviceSize offset = 0) const;
		void* map(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize mapSize = VK_WHOLE_SIZE) const;
		void unmap(VkDevice device) const ;
		void write(VkDevice device, const void* srcData, VkDeviceSize writeSize, VkDeviceSize offset = 0) const;

		void flush(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize flushSize = VK_WHOLE_SIZE) const;
		void invalidate(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize invalidateSize = VK_WHOLE_SIZE) const;
		static void copy(const VulkanContext& ctx, const Buffer& src, Buffer& dst, VkDeviceSize copySize);

		bool isHostVisible() const noexcept { return (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }

		static Buffer createVertexBuffer(const VulkanContext& ctx, const void* data, VkDeviceSize size);
		static Buffer createIndexBuffer(const VulkanContext& ctx, const void* data, VkDeviceSize size);
		static Buffer createUniformBuffer(const VulkanContext& ctx, VkDeviceSize size);

		uint32_t findMemoryType(const VulkanContext& context, uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
	};
}
