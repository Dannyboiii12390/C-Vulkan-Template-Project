#include "Buffer.h"
#include "../VulkanContext.h"
#include <stdexcept>
#include <cstring>
namespace Engine
{
    void Buffer::create(VulkanContext& ctx, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkMemoryPropertyFlags memProperties) {
        size = bufferSize;
        usage = bufferUsage;
        properties = memProperties;

        VkDevice device = ctx.getDevice();

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(ctx, memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &memory) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate buffer memory");
        }

        if (vkBindBufferMemory(device, buffer, memory, 0) != VK_SUCCESS) {
            throw std::runtime_error("Failed to bind buffer memory");
        }
    }
    void Buffer::destroy(VkDevice device) {
        if (buffer != VK_NULL_HANDLE) {
            vkDestroyBuffer(device, buffer, nullptr);
            buffer = VK_NULL_HANDLE;
        }
        if (memory != VK_NULL_HANDLE) {
            vkFreeMemory(device, memory, nullptr);
            memory = VK_NULL_HANDLE;
        }
        size = 0;
        usage = 0;
        properties = 0;
    }
    void Buffer::bind(VkDevice device, VkDeviceSize offset) {
        if (buffer != VK_NULL_HANDLE && memory != VK_NULL_HANDLE) {
            vkBindBufferMemory(device, buffer, memory, offset);
        }
    }
    void* Buffer::map(VkDevice device, VkDeviceSize offset, VkDeviceSize mapSize) {
        void* mapped = nullptr;
        if (vkMapMemory(device, memory, offset, mapSize, 0, &mapped) != VK_SUCCESS) {
            throw std::runtime_error("Failed to map buffer memory");
        }
        return mapped;
    }
    void Buffer::unmap(VkDevice device) {
        if (memory != VK_NULL_HANDLE) {
            vkUnmapMemory(device, memory);
        }
    }
    void Buffer::write(VkDevice device, const void* srcData, VkDeviceSize writeSize, VkDeviceSize offset) {
        if (writeSize == 0 || srcData == nullptr) return;
        void* dst = map(device, offset, writeSize);
        std::memcpy(dst, srcData, static_cast<size_t>(writeSize));
        if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
            flush(device, offset, writeSize);
        }
        unmap(device);
    }
    void Buffer::flush(VkDevice device, VkDeviceSize offset, VkDeviceSize flushSize) {
        if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0) return; // not required
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = memory;
        range.offset = offset;
        range.size = flushSize;
        vkFlushMappedMemoryRanges(device, 1, &range);
    }
    void Buffer::invalidate(VkDevice device, VkDeviceSize offset, VkDeviceSize invalidateSize) {
        if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) != 0) return;
        VkMappedMemoryRange range{};
        range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
        range.memory = memory;
        range.offset = offset;
        range.size = invalidateSize;
        vkInvalidateMappedMemoryRanges(device, 1, &range);
    }
    void Buffer::copy(VulkanContext& context, const Buffer& src, Buffer& dst, VkDeviceSize copySize) {
        if (copySize == 0) copySize = src.size;
        //ctx.copyBuffer(src.buffer, dst.buffer, copySize);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = context.getCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(context.getDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        VkBufferCopy copyRegion{};
        copyRegion.size = copySize;
        vkCmdCopyBuffer(commandBuffer, src.buffer, dst.buffer, 1, &copyRegion);
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.getGraphicsQueue());
        vkFreeCommandBuffers(context.getDevice(), context.getCommandPool(), 1, &commandBuffer);
    }

    Buffer Buffer::createVertexBuffer(VulkanContext& ctx, const void* data, VkDeviceSize dataSize) {
        // Create staging buffer
        Buffer staging{};
        staging.create(ctx, dataSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        staging.write(ctx.getDevice(), data, dataSize);

        // Create device local vertex buffer
        Buffer vertex{};
        vertex.create(ctx, dataSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        // Copy staging -> device local
        copy(ctx, staging, vertex, dataSize);

        // Cleanup staging
        staging.destroy(ctx.getDevice());

        return vertex;
    }
    Buffer Buffer::createIndexBuffer(VulkanContext& ctx, const void* data, VkDeviceSize dataSize) {
        // Reuse the same flow as vertex: staging -> device local
        Buffer staging{};
        staging.create(ctx, dataSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        staging.write(ctx.getDevice(), data, dataSize);

        Buffer index{};
        index.create(ctx, dataSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        copy(ctx, staging, index, dataSize);
        staging.destroy(ctx.getDevice());

        return index;
    }
    Buffer Buffer::createUniformBuffer(VulkanContext& ctx, VkDeviceSize dataSize) {
        Buffer ub{};
        ub.create(ctx, dataSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        return ub;
    }

    uint32_t Buffer::findMemoryType(VulkanContext& context, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(context.getPhysicalDevice(), &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        ASSERT(1 != 0); // Failed to find suitable memory type!
    }
}