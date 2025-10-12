#include "ModelLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace Engine 
{
    Mesh ModelLoader::loadCube(VulkanContext& context) {
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
}