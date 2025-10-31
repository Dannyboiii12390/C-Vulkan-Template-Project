#include "ModelLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../VulkanContext.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine
{
    Mesh ModelLoader::createCube(VulkanContext& context)
    {
        std::vector<Vertex> vertices{
            // Front face
            {{-0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}},

            // Back face
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}}
        };
        std::vector<uint16_t> indices = {
            // Front face
            0, 1, 2, 2, 3, 0,
            // Back face
            4, 5, 6, 6, 7, 4,
            // Top face
            3, 2, 6, 6, 7, 3,
            // Bottom face
            0, 1, 5, 5, 4, 0,
            // Right face
            1, 2, 6, 6, 5, 1,
            // Left face
            0, 3, 7, 7, 4, 0
        };
        //moving values into the mesh
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    };
    Mesh ModelLoader::createCubeWithoutIndex(VulkanContext& context)
    {
        std::vector<Vertex> vertices{
            // --- Front face (Z+) - 1x1 (Single coin)
            // Normal: (0, 0, 1)
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}}, // Bottom-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top-right

            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}, // Bottom-left
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}}, // Top-right
            {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // Top-left

            // --- Back face (Z-) - 2x2 (Four coins)
            // Normal: (0, 0, -1)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 2.0f}}, // Bottom-right
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // Top-right
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {2.0f, 0.0f}}, // Top-left

            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 2.0f}}, // Bottom-right
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {2.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {2.0f, 2.0f}}, // Bottom-left

            // --- Left face (X-) - 3x3 (Nine coins)
            // Normal: (-1, 0, 0)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 3.0f}}, // Bottom-left
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {3.0f, 3.0f}}, // Bottom-right
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {3.0f, 0.0f}}, // Top-right

            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 3.0f}}, // Bottom-left
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {3.0f, 0.0f}}, // Top-right
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Top-left

            // --- Right face (X+) - 4x4 (Sixteen coins)
            // Normal: (1, 0, 0)
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 4.0f}}, // Bottom-right
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Top-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {4.0f, 0.0f}}, // Top-left

            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 4.0f}}, // Bottom-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {4.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {4.0f, 4.0f}}, // Bottom-left

            // --- Top face (Y+) - 2x3 (Six coins)
            // Normal: (0, 1, 0)
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 3.0f}}, // Bottom-left
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}, // Top-left
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}}, // Top-right

            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 3.0f}}, // Bottom-left
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 0.0f}}, // Top-right
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {2.0f, 3.0f}}, // Bottom-right

            // --- Bottom face (Y-) - 3x2 (Six coins)
            // Normal: (0, -1, 0)
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {3.0f, 0.0f}}, // Top-right
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {3.0f, 2.0f}}, // Bottom-right

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {3.0f, 2.0f}}, // Bottom-right
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 2.0f}}, // Bottom-left
        };

        Mesh mesh;
        mesh.create(context, std::move(vertices));
        return mesh;

    };
    Mesh ModelLoader::createGrid(VulkanContext& context, int width, int depth)
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        int halfWidth = width / 2;
        int halfDepth = depth / 2;

        // Generate vertices
        for (int z = -halfDepth; z <= halfDepth; ++z)
        {
            for (int x = -halfWidth; x <= halfWidth; ++x)
            {
                Engine::Vertex v{};
                v.pos = { glm::vec3(static_cast<float>(x), 0.0f, static_cast<float>(z)) };
                v.color = { 1.0f,1.0f,1.0f };
                vertices.push_back(v);
            }
        }

        // Generate indices for line segments (horizontal and vertical lines)
        for (int z = 0; z <= depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                uint32_t start = z * (width + 1) + x;
                indices.push_back(start);
                indices.push_back(start + 1);
            }
        }

        for (int x = 0; x <= width; ++x)
        {
            for (int z = 0; z < depth; ++z)
            {
                uint32_t start = z * (width + 1) + x;
                indices.push_back(start);
                indices.push_back(start + (width + 1));
            }
        }
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;

    }
    Mesh ModelLoader::createTerrain(VulkanContext& context, int width, int depth, float cellSize)
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        int halfWidth = width / 2;
        int halfDepth = depth / 2;

        // Generate vertices
        for (int z = -halfDepth; z <= halfDepth; ++z)
        {
            for (int x = -halfWidth; x <= halfWidth; ++x)
            {
                Engine::Vertex v{};
                float y = static_cast<float>(sin(x) * cos(z)); // Simple height function for terrain
                v.pos = { glm::vec3(static_cast<float>(x), y, static_cast<float>(z)) };
                v.color = { 1.0f,1.0f,1.0f };
                vertices.push_back(v);
            }
        }

        // Generate indices for line segments (horizontal and vertical lines)
        for (int z = 0; z <= depth; ++z)
        {
            for (int x = 0; x < width; ++x)
            {
                uint32_t start = z * (width + 1) + x;
                indices.push_back(start);
                indices.push_back(start + 1);
            }
        }

        for (int x = 0; x <= width; ++x)
        {
            for (int z = 0; z < depth; ++z)
            {
                uint32_t start = z * (width + 1) + x;
                indices.push_back(start);
                indices.push_back(start + (width + 1));
            }
        }
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    }
    Mesh ModelLoader::createCylinder(VulkanContext& context, float radius, float height, int segmentCount)
    {
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        float halfHeight = height / 2.0f;
        float angleStep = glm::two_pi<float>() / segmentCount;

        // === Bottom cap vertices ===
        uint32_t bottomCenterIndex = static_cast<uint32_t>(vertices.size());
        vertices.push_back({ {0.0f, -halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f} });  // center vertex

        for (int i = 0; i <= segmentCount; ++i)
        {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back({ {x, -halfHeight, z}, {1.0f, 0.0f, 0.0f} });
        }

        // === Top cap vertices ===
        uint32_t topCenterIndex = static_cast<uint32_t>(vertices.size());
        vertices.push_back({ {0.0f, +halfHeight, 0.0f}, {1.0f, 1.0f, 1.0f} });  // center vertex

        for (int i = 0; i <= segmentCount; ++i)
        {
            float angle = i * angleStep;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back({ {x, +halfHeight, z}, {0.0f, 0.0f, 1.0f} });
        }

        // === Indices for bottom cap ===
        for (int i = 1; i <= segmentCount; ++i)
        {
            indices.push_back(bottomCenterIndex);
            indices.push_back(bottomCenterIndex + i);
            indices.push_back(bottomCenterIndex + i + 1);
        }

        // === Indices for top cap ===
        for (int i = 1; i <= segmentCount; ++i)
        {
            indices.push_back(topCenterIndex);
            indices.push_back(topCenterIndex + i + 1);
            indices.push_back(topCenterIndex + i);
        }

        // === Indices for side walls ===
        uint32_t bottomStart = bottomCenterIndex + 1;
        uint32_t topStart = topCenterIndex + 1;

        for (int i = 0; i < segmentCount; ++i)
        {
            uint32_t bl = bottomStart + i;
            uint32_t br = bottomStart + i + 1;
            uint32_t tl = topStart + i;
            uint32_t tr = topStart + i + 1;

            // First triangle
            indices.push_back(bl);
            indices.push_back(tl);
            indices.push_back(tr);

            // Second triangle
            indices.push_back(bl);
            indices.push_back(tr);
            indices.push_back(br);
        }
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    }
    Mesh ModelLoader::loadObj(VulkanContext& context, const char* filepath)
    {
        //load the model 
        Assimp::Importer importer;

        const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << std::endl;
        }

        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        // Get first mesh
        aiMesh* aimesh = scene->mMeshes[0];

        // === Vertices ===
        for (unsigned int i = 0; i < aimesh->mNumVertices; ++i)
        {
            Engine::Vertex vertex{};

            vertex.pos = glm::vec3(
                aimesh->mVertices[i].x,
                aimesh->mVertices[i].y,
                aimesh->mVertices[i].z
            );
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // Default white color

            vertices.push_back(vertex);
        }

        // === Indices ===
        for (unsigned int i = 0; i < aimesh->mNumFaces; ++i)
        {
            aiFace face = aimesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; ++j)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;

    }
    Mesh ModelLoader::createSphere(VulkanContext& context, float radius, int sectorCount, int stackCount)
    {


        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        const float PI = 3.14159265359f;

        for (int i = 0; i <= stackCount; ++i)
        {
            float stackAngle = PI / 2 - i * (PI / stackCount);  // from pi/2 to -pi/2
            float xy = radius * cosf(stackAngle);               // r * cos(u)
            float z = radius * sinf(stackAngle);                // r * sin(u)

            for (int j = 0; j <= sectorCount; ++j)
            {
                float sectorAngle = j * (2 * PI / sectorCount); // from 0 to 2pi

                float x = xy * cosf(sectorAngle);               // r * cos(u) * cos(v)
                float y = xy * sinf(sectorAngle);               // r * cos(u) * sin(v)

                glm::vec3 position(x, y, z);
                glm::vec3 normal = glm::vec3(0.5f) - glm::normalize(position);
                //glm::vec2 texCoord((float)j / sectorCount, (float)i / stackCount);

                vertices.emplace_back(position, normal);
            }
        }

        for (int i = 0; i < stackCount; ++i)
        {
            int k1 = i * (sectorCount + 1);     // beginning of current stack
            int k2 = k1 + sectorCount + 1;      // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                if (i != 0)
                {
                    indices.push_back(k1);
                    indices.push_back(k2);
                    indices.push_back(k1 + 1);
                }

                if (i != (stackCount - 1))
                {
                    indices.push_back(k1 + 1);
                    indices.push_back(k2);
                    indices.push_back(k2 + 1);
                }
            }
        }
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;

    }

    Texture ModelLoader::createTextureImage(VulkanContext& context, const char* filepath)
    {
        Texture texture{};
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) throw std::runtime_error("Failed to load texture image!");
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        // Create staging buffer and upload pixels
        Engine::Buffer stagingBuffer;
        stagingBuffer.create(context, imageSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.write(context.getDevice(), pixels, imageSize);
        stbi_image_free(pixels);

        // Create image
        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { (uint32_t)texWidth, (uint32_t)texHeight, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(context.getDevice(), &imageInfo, nullptr, &textureImage) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(context.getDevice(), textureImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = stagingBuffer.findMemoryType(context, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate texture image memory!");

        vkBindImageMemory(context.getDevice(), textureImage, textureImageMemory, 0);

        // Transition, copy, transition
        transitionImageLayout(context, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer.buffer, textureImage, (uint32_t)texWidth, (uint32_t)texHeight);

        transitionImageLayout(context, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // staging no longer needed
        stagingBuffer.destroy(context.getDevice());

        VkImageView textureImageView = createImageView(context, textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        VkSampler textureSampler = createTextureSampler(context);

        // IMPORTANT: store textureImage and textureImageMemory somewhere to free on cleanup (e.g., in VulkanContext or a resource manager)
        texture.image = textureImage;
        texture.imageMemory = textureImageMemory;
        texture.imageView = textureImageView;
        texture.nearestSampler = createNearestSampler(context);
        texture.bilinearSampler = createBilinearSampler(context);
        texture.trilinearSampler = createTrilinearSampler(context);
        texture.anisotropicSampler = createAnisotropicSampler(context);
        texture.sampler = textureSampler;

        return texture;
    }
    void ModelLoader::transitionImageLayout(VulkanContext& context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("Unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(context, commandBuffer);
    }
    void ModelLoader::copyBufferToImage(VulkanContext& context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(context);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(context, commandBuffer);
    }
    VkCommandBuffer ModelLoader::beginSingleTimeCommands(VulkanContext& context)
    {
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
        return commandBuffer;
    }
    void ModelLoader::endSingleTimeCommands(VulkanContext& context, VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(context.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(context.getGraphicsQueue());

        vkFreeCommandBuffers(context.getDevice(), context.getCommandPool(), 1, &commandBuffer);
    }
    VkImageView ModelLoader::createImageView(VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(context.getDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture image view!");
        }

        return imageView;
    }

    VkSampler ModelLoader::createTextureSampler(VulkanContext& context)
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(context.getPhysicalDevice(), &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createNearestSampler(VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Nearest-neighbor filtering (no interpolation)
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create nearest sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createBilinearSampler(VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Bilinear filtering (linear interpolation)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // Nearest mipmap mode for bilinear (not trilinear)
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create bilinear sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createTrilinearSampler(VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Trilinear filtering (linear + mipmap interpolation)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        // Linear mipmap mode for trilinear
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 10.0f; // Allow multiple mipmap levels

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create trilinear sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createAnisotropicSampler(VulkanContext& context, float maxAnisotropy)
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(context.getPhysicalDevice(), &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Anisotropic filtering (best quality for oblique viewing angles)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        // Enable anisotropic filtering if supported
        samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy ? VK_TRUE : VK_FALSE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = deviceFeatures.samplerAnisotropy ?
            std::min(maxAnisotropy, properties.limits.maxSamplerAnisotropy) : 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 10.0f;

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create anisotropic sampler!");
        }

        return textureSampler;
    }
}