#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>

class VulkanContext; // forward

// Lightweight RAII-style wrapper for VkBuffer + VkDeviceMemory.
// Designed to be used with the existing `VulkanContext` in this project.
class Buffer {
public:
    // Vulkan handles
    VkBuffer buffer = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;

    // Size and usage metadata
    VkDeviceSize size = 0;
    VkBufferUsageFlags usage = 0;
    VkMemoryPropertyFlags properties = 0;

    Buffer() = default;
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) noexcept = default;
    Buffer& operator=(Buffer&&) noexcept = default;
    ~Buffer() = default; // explicit destroy required via destroy(device)

    // Create a buffer and allocate memory for it using the VulkanContext.
    // Throws std::runtime_error on failure.
    void create(class VulkanContext& ctx, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

    // Free buffer and memory. Caller must pass the logical device used to create the buffer.
    void destroy(VkDevice device);

    // Bind memory (wraps vkBindBufferMemory but left public for explicit use)
    void bind(VkDevice device, VkDeviceSize offset = 0);

    // Map and unmap
    // Returns pointer to mapped memory (must not be used after unmap).
    void* map(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize mapSize = VK_WHOLE_SIZE);
    
    void unmap(VkDevice device);

    // Write raw data into the buffer (maps/unmaps internally).
    void write(VkDevice device, const void* srcData, VkDeviceSize writeSize, VkDeviceSize offset = 0);

    // Flush or invalidate ranges for non-coherent memory
    void flush(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize flushSize = VK_WHOLE_SIZE);
    void invalidate(VkDevice device, VkDeviceSize offset = 0, VkDeviceSize invalidateSize = VK_WHOLE_SIZE);

    // Convenience check
    bool isHostVisible() const noexcept { return (properties & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0; }

    // Copy helper that leverages VulkanContext::copyBuffer implementation.
    static void copy(class VulkanContext& ctx, const Buffer& src, Buffer& dst, VkDeviceSize copySize);

    // Factory helpers that create and return fully initialized buffers.
    // Vertex/index factories create an optimal DEVICE_LOCAL buffer and use a staging buffer + ctx.copyBuffer.
    static Buffer createVertexBuffer(class VulkanContext& ctx, const void* data, VkDeviceSize size);
    static Buffer createIndexBuffer(class VulkanContext& ctx, const void* data, VkDeviceSize size);
    // Uniform buffer helper; keeps memory host visible for frequent updates.
    static Buffer createUniformBuffer(class VulkanContext& ctx, VkDeviceSize size);
};