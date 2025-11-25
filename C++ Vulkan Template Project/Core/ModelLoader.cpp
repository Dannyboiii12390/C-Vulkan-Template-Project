#include "ModelLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <map>
#include <sstream>

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
            // --- Front face (Z+)
            // Normal: (0, 0, 1)  Tangent: (1,0,0) Binormal: (0,1,0)
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right

            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right
            {{-0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left

            // --- Back face (Z-)
            // Normal: (0, 0, -1)  Tangent: (-1,0,0) Binormal: (0,1,0)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left

            {{-0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left

            // --- Left face (X-)
            // Normal: (-1, 0, 0)  Tangent: (0,0,1) Binormal: (0,1,0)
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right

            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left

            // --- Right face (X+)
            // Normal: (1, 0, 0)  Tangent: (0,0,-1) Binormal: (0,1,0)
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left

            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-right
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Top-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}}, // Bottom-left
            
            // --- Top face (Y+)
            // Normal: (0, 1, 0)  Tangent: (1,0,0) Binormal: (0,0,-1)
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Front-left (top-left)
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Front-right (top-right)
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Back-right (top-right)

            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Back-right (top-right)
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Back-left (top-left)
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}}, // Front-left (top-left)

            // --- Bottom face (Y-)
            // Normal: (0, -1, 0)  Tangent: (1,0,0) Binormal: (0,0,1)
            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top-left
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top-right
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-right

            {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Top-left
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-right
            {{-0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Bottom-left
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
    Mesh ModelLoader::createTerrain(VulkanContext& context, int totalWidth, int totalDepth, float cellSize, float UVsize)
    {
        std::vector<Vertex> vertices;
        // Validate inputs
        if (cellSize <= 0.0f) cellSize = 1.0f;
        if (totalWidth <= 0) totalWidth = 1;
        if (totalDepth <= 0) totalDepth = 1;

        // Interpret 'totalWidth' and 'totalDepth' as the full world extents (in world units).
        // 'cellSize' is the size of one cell (triangle square side length).
        // Compute number of cells along each axis (at least 1).
        int cellsX = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalWidth) / cellSize)));
        int cellsZ = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalDepth) / cellSize)));

        // Number of vertices along each axis
        const int vertsX = cellsX + 1;
        const int vertsZ = cellsZ + 1;

        // World-space half extents so the terrain is centered around the origin
        const float halfWidth = (cellsX * cellSize) * 0.5f;
        const float halfDepth = (cellsZ * cellSize) * 0.5f;

        // Height function in world coordinates (can be replaced by a heightmap lookup)
        auto heightAt = [&](int ix, int iz) -> float {
            float wx = (static_cast<float>(ix) * cellSize) - halfWidth;
            float wz = (static_cast<float>(iz) * cellSize) - halfDepth;
            return std::sinf(wx) * std::cosf(wz);
            };

        // Reserve memory: two triangles (6 vertices) per cell
        vertices.reserve(static_cast<size_t>(cellsX) * static_cast<size_t>(cellsZ) * 6);

        // Helper to compute world position for grid coordinate (ix,iz) where ix in [0..cellsX], iz in [0..cellsZ]
        auto positionAt = [&](int ix, int iz) -> glm::vec3 {
            float wx = (static_cast<float>(ix) * cellSize) - halfWidth;
            float wz = (static_cast<float>(iz) * cellSize) - halfDepth;
            float y = heightAt(ix, iz);
            return glm::vec3(wx, y, wz);
            };

        // Generate non-indexed triangles (two triangles per quad cell).
        for (int iz = 0; iz < cellsZ; ++iz)
        {
            for (int ix = 0; ix < cellsX; ++ix)
            {
                // Quad corners (a = bottom-left, b = bottom-right, c = top-left, d = top-right)
                glm::vec3 aPos = positionAt(ix, iz);
                glm::vec3 bPos = positionAt(ix + 1, iz);
                glm::vec3 cPos = positionAt(ix, iz + 1);
                glm::vec3 dPos = positionAt(ix + 1, iz + 1);

                // Texture coordinates span 0..1 across the whole terrain (use cell index / cell count)
                glm::vec2 aUV = glm::vec2(static_cast<float>(ix) / static_cast<float>(cellsX), static_cast<float>(iz) / static_cast<float>(cellsZ)) * static_cast<float>(UVsize);
                glm::vec2 bUV = glm::vec2(static_cast<float>(ix + 1) / static_cast<float>(cellsX), static_cast<float>(iz) / static_cast<float>(cellsZ)) * static_cast<float>(UVsize);
                glm::vec2 cUV = glm::vec2(static_cast<float>(ix) / static_cast<float>(cellsX), static_cast<float>(iz + 1) / static_cast<float>(cellsZ)) * static_cast<float>(UVsize);
                glm::vec2 dUV = glm::vec2(static_cast<float>(ix + 1) / static_cast<float>(cellsX), static_cast<float>(iz + 1) / static_cast<float>(cellsZ)) * static_cast<float>(UVsize);


                // First triangle: a, c, d
                glm::vec3 n1 = glm::normalize(glm::cross(cPos - aPos, dPos - aPos));
                {
                    Vertex va{};
                    va.pos = aPos;
                    va.normal = n1;
                    va.texCoord = aUV;
                    va.color = glm::vec3(1.0f);
                    vertices.push_back(va);

                    Vertex vc{};
                    vc.pos = cPos;
                    vc.normal = n1;
                    vc.texCoord = cUV;
                    vc.color = glm::vec3(1.0f);
                    vertices.push_back(vc);

                    Vertex vd{};
                    vd.pos = dPos;
                    vd.normal = n1;
                    vd.texCoord = dUV;
                    vd.color = glm::vec3(1.0f);
                    vertices.push_back(vd);
                }

                // Second triangle: a, d, b
                glm::vec3 n2 = glm::normalize(glm::cross(dPos - aPos, bPos - aPos));
                {
                    Vertex va2{};
                    va2.pos = aPos;
                    va2.normal = n2;
                    va2.texCoord = aUV;
                    va2.color = glm::vec3(1.0f);
                    vertices.push_back(va2);

                    Vertex vd2{};
                    vd2.pos = dPos;
                    vd2.normal = n2;
                    vd2.texCoord = dUV;
                    vd2.color = glm::vec3(1.0f);
                    vertices.push_back(vd2);

                    Vertex vb{};
                    vb.pos = bPos;
                    vb.normal = n2;
                    vb.texCoord = bUV;
                    vb.color = glm::vec3(1.0f);
                    vertices.push_back(vb);
                }
            }
        }

        Mesh mesh;
        // Use the overload that creates a vertex-only (non-indexed) mesh
        mesh.create(context, std::move(vertices));
        return mesh;
    }
    Mesh ModelLoader::createCylinder(VulkanContext & context, float radius, float height, int segmentCount, float UVsize)
    {
        std::vector<Vertex> vertices;

        if (segmentCount < 3) segmentCount = 3;
        if (radius <= 0.0f) radius = 1.0f;
        if (UVsize <= 0.0f) UVsize = 1.0f;

        const float halfHeight = height * 0.5f;
        const float angleStep = glm::two_pi<float>() / static_cast<float>(segmentCount);

        // Precompute rim positions (include wrap-around vertex at the end)
        std::vector<glm::vec3> rimPos(segmentCount + 1);
        for (int i = 0; i <= segmentCount; ++i) {
            float a = i * angleStep;
            rimPos[i] = glm::vec3(radius * std::cos(a), 0.0f, radius * std::sin(a));
        }

        // --- Bottom cap (non-indexed) ---
        {
            glm::vec3 normal = glm::vec3(0.0f, -1.0f, 0.0f);
            // center UV
            glm::vec2 centerUV = glm::vec2(0.5f, 0.5f) * UVsize;
            for (int i = 0; i < segmentCount; ++i) {
                // Triangle: center, rim[i+1], rim[i]  (matches previous winding)
                Vertex vc{};
                vc.pos = glm::vec3(0.0f, -halfHeight, 0.0f);
                vc.normal = normal;
                vc.color = glm::vec3(1.0f);
                vc.texCoord = centerUV;
                vc.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                vc.binormal = glm::cross(normal, vc.tangent);
                vertices.push_back(vc);

                auto p1 = rimPos[i + 1];
                Vertex v1{};
                v1.pos = glm::vec3(p1.x, -halfHeight, p1.z);
                v1.normal = normal;
                v1.color = glm::vec3(1.0f);
                v1.texCoord = (glm::vec2((p1.x / radius) * 0.5f + 0.5f, (p1.z / radius) * 0.5f + 0.5f)) * UVsize;
                v1.tangent = glm::normalize(glm::vec3(std::cos((i + 1) * angleStep), 0.0f, std::sin((i + 1) * angleStep)));
                v1.binormal = glm::cross(normal, v1.tangent);
                vertices.push_back(v1);

                auto p0 = rimPos[i];
                Vertex v0{};
                v0.pos = glm::vec3(p0.x, -halfHeight, p0.z);
                v0.normal = normal;
                v0.color = glm::vec3(1.0f);
                v0.texCoord = (glm::vec2((p0.x / radius) * 0.5f + 0.5f, (p0.z / radius) * 0.5f + 0.5f)) * UVsize;
                v0.tangent = glm::normalize(glm::vec3(std::cos(i * angleStep), 0.0f, std::sin(i * angleStep)));
                v0.binormal = glm::cross(normal, v0.tangent);
                vertices.push_back(v0);
            }
        }

        // --- Top cap (non-indexed) ---
        {
            glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec2 centerUV = glm::vec2(0.5f, 0.5f) * UVsize;
            for (int i = 0; i < segmentCount; ++i) {
                // Triangle: center, rim[i], rim[i+1] (winding consistent)
                Vertex vc{};
                vc.pos = glm::vec3(0.0f, +halfHeight, 0.0f);
                vc.normal = normal;
                vc.color = glm::vec3(1.0f);
                vc.texCoord = centerUV;
                vc.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                vc.binormal = glm::cross(normal, vc.tangent);
                vertices.push_back(vc);

                auto p0 = rimPos[i];
                Vertex v0{};
                v0.pos = glm::vec3(p0.x, +halfHeight, p0.z);
                v0.normal = normal;
                v0.color = glm::vec3(1.0f);
                v0.texCoord = (glm::vec2((p0.x / radius) * 0.5f + 0.5f, (p0.z / radius) * 0.5f + 0.5f)) * UVsize;
                v0.tangent = glm::normalize(glm::vec3(std::cos(i * angleStep), 0.0f, std::sin(i * angleStep)));
                v0.binormal = glm::cross(normal, v0.tangent);
                vertices.push_back(v0);

                auto p1 = rimPos[i + 1];
                Vertex v1{};
                v1.pos = glm::vec3(p1.x, +halfHeight, p1.z);
                v1.normal = normal;
                v1.color = glm::vec3(1.0f);
                v1.texCoord = (glm::vec2((p1.x / radius) * 0.5f + 0.5f, (p1.z / radius) * 0.5f + 0.5f)) * UVsize;
                v1.tangent = glm::normalize(glm::vec3(std::cos((i + 1) * angleStep), 0.0f, std::sin((i + 1) * angleStep)));
                v1.binormal = glm::cross(normal, v1.tangent);
                vertices.push_back(v1);
            }
        }

        // --- Side wall (non-indexed) ---
        // Build two triangles per segment; side normals are radial; UV.u spans [0..1]*UVsize around, v is 0..1 * UVsize
        for (int i = 0; i < segmentCount; ++i) {
            float a0 = i * angleStep;
            float a1 = (i + 1) * angleStep;

            glm::vec3 p0_bottom = glm::vec3(radius * std::cos(a0), -halfHeight, radius * std::sin(a0));
            glm::vec3 p1_bottom = glm::vec3(radius * std::cos(a1), -halfHeight, radius * std::sin(a1));
            glm::vec3 p0_top = glm::vec3(radius * std::cos(a0), +halfHeight, radius * std::sin(a0));
            glm::vec3 p1_top = glm::vec3(radius * std::cos(a1), +halfHeight, radius * std::sin(a1));

            glm::vec3 radial0 = glm::normalize(glm::vec3(p0_bottom.x, 0.0f, p0_bottom.z));
            glm::vec3 radial1 = glm::normalize(glm::vec3(p1_bottom.x, 0.0f, p1_bottom.z));

            // Tangent direction along circumferential direction
            glm::vec3 tangent0 = glm::normalize(glm::vec3(-std::sin(a0), 0.0f, std::cos(a0)));
            glm::vec3 tangent1 = glm::normalize(glm::vec3(-std::sin(a1), 0.0f, std::cos(a1)));

            float u0 = (static_cast<float>(i) / static_cast<float>(segmentCount)) * UVsize;
            float u1 = (static_cast<float>(i + 1) / static_cast<float>(segmentCount)) * UVsize;
            float v0 = 0.0f * UVsize;
            float v1 = 1.0f * UVsize;

            // First triangle (bottom p0, top p0, top p1)
            {
                Vertex vb{};
                vb.pos = p0_bottom;
                vb.normal = radial0;
                vb.color = glm::vec3(1.0f);
                vb.texCoord = glm::vec2(u0, v0);
                vb.tangent = tangent0;
                vb.binormal = glm::cross(radial0, tangent0);
                vertices.push_back(vb);

                Vertex vt0{};
                vt0.pos = p0_top;
                vt0.normal = radial0;
                vt0.color = glm::vec3(1.0f);
                vt0.texCoord = glm::vec2(u0, v1);
                vt0.tangent = tangent0;
                vt0.binormal = glm::cross(radial0, tangent0);
                vertices.push_back(vt0);

                Vertex vt1{};
                vt1.pos = p1_top;
                vt1.normal = radial1;
                vt1.color = glm::vec3(1.0f);
                vt1.texCoord = glm::vec2(u1, v1);
                vt1.tangent = tangent1;
                vt1.binormal = glm::cross(radial1, tangent1);
                vertices.push_back(vt1);
            }

            // Second triangle (bottom p0, top p1, bottom p1)
            {
                Vertex vb{};
                vb.pos = p0_bottom;
                vb.normal = radial0;
                vb.color = glm::vec3(1.0f);
                vb.texCoord = glm::vec2(u0, v0);
                vb.tangent = tangent0;
                vb.binormal = glm::cross(radial0, tangent0);
                vertices.push_back(vb);

                Vertex vt1{};
                vt1.pos = p1_top;
                vt1.normal = radial1;
                vt1.color = glm::vec3(1.0f);
                vt1.texCoord = glm::vec2(u1, v1);
                vt1.tangent = tangent1;
                vt1.binormal = glm::cross(radial1, tangent1);
                vertices.push_back(vt1);

                Vertex vb1{};
                vb1.pos = p1_bottom;
                vb1.normal = radial1;
                vb1.color = glm::vec3(1.0f);
                vb1.texCoord = glm::vec2(u1, v0);
                vb1.tangent = tangent1;
                vb1.binormal = glm::cross(radial1, tangent1);
                vertices.push_back(vb1);
            }
        }

        Mesh mesh;
        // Non-indexed mesh creation
        mesh.create(context, std::move(vertices));
        return mesh;
    }
    
    Mesh ModelLoader::loadOBJ(VulkanContext& context, const char* filepath, float UVsize)
    {
        // Pseudocode:
        // 1. Open the file and read all lines.
        // 2. Parse vertex positions (v), texture coordinates (vt), normals (vn).
        // 3. Parse faces (f) and build indices.
        // 4. For each face, create Vertex objects (pos, normal, texCoord, color, tangent, binormal).
        // 5. Store vertices and indices, avoiding duplicates (vertex deduplication).
        // 6. Create Mesh using mesh.create(context, vertices, indices).

        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error(std::string("Failed to open OBJ file: ") + filepath);
        }

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;
        struct Index { int v, vt, vn; bool operator<(const Index& o) const { return std::tie(v, vt, vn) < std::tie(o.v, o.vt, o.vn); } };
        std::map<Index, uint16_t> indexMap;
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if (prefix == "v") {
                float x, y, z;
                iss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
            else if (prefix == "vt") {
                float u, v;
                iss >> u >> v;
                texCoords.emplace_back(u * UVsize, v * UVsize);
            }
            else if (prefix == "vn") {
                float nx, ny, nz;
                iss >> nx >> ny >> nz;
                normals.emplace_back(nx, ny, nz);
            }
            else if (prefix == "f") {
                std::vector<Index> faceIndices;
                std::string vertStr;
                while (iss >> vertStr) {
                    int v = 0, vt = 0, vn = 0;
                    size_t p1 = vertStr.find('/');
                    size_t p2 = vertStr.find('/', p1 + 1);
                    v = std::stoi(vertStr.substr(0, p1)) - 1;
                    if (p1 != std::string::npos && p2 != std::string::npos && p2 > p1 + 1)
                        vt = std::stoi(vertStr.substr(p1 + 1, p2 - p1 - 1)) - 1;
                    else
                        vt = -1;
                    if (p2 != std::string::npos)
                        vn = std::stoi(vertStr.substr(p2 + 1)) - 1;
                    else
                        vn = -1;
                    faceIndices.push_back({ v, vt, vn });
                }
                // Triangulate polygons (assume convex)
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                    Index idxs[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };
                    for (int k = 0; k < 3; ++k) {
                        auto it = indexMap.find(idxs[k]);
                        if (it == indexMap.end()) {
                            Vertex vert{};
                            vert.pos = positions[idxs[k].v];
                            vert.texCoord = (idxs[k].vt >= 0 && idxs[k].vt < (int)texCoords.size()) ? texCoords[idxs[k].vt] : glm::vec2(0.0f);
                            vert.normal = (idxs[k].vn >= 0 && idxs[k].vn < (int)normals.size()) ? normals[idxs[k].vn] : glm::vec3(0.0f, 1.0f, 0.0f);
                            vert.color = glm::vec3(1.0f);
                            // Tangent/binormal: simple orthonormal basis
                            glm::vec3 up = glm::abs(vert.normal.y) < 0.999f ? glm::vec3(0.0f, 1.0f, 0.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                            vert.tangent = glm::normalize(glm::cross(up, vert.normal));
                            vert.binormal = glm::normalize(glm::cross(vert.normal, vert.tangent));
                            uint16_t idx = static_cast<uint16_t>(vertices.size());
                            vertices.push_back(vert);
                            indexMap[idxs[k]] = idx;
                            indices.push_back(idx);
                        }
                        else {
                            indices.push_back(it->second);
                        }
                    }
                }
            }
        }
        file.close();

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
    Engine::Mesh ModelLoader::createParticleSystem(VulkanContext& context, int particleCount) {
        std::vector<Vertex> vertices;
        vertices.reserve(particleCount);

        // Create random particle positions
        for (uint32_t i = 0; i < particleCount; ++i) {
            Vertex v{};
            // Random position in a volume
            v.pos = glm::vec3(
                (rand() / (float)RAND_MAX - 0.5f) * 2.0f,
                (rand() / (float)RAND_MAX - 0.5f) * 2.0f,
                (rand() / (float)RAND_MAX - 0.5f) * 2.0f
            );
            v.normal = glm::normalize(v.pos);  // Use as velocity direction
            v.texCoord = glm::vec2(rand() / (float)RAND_MAX, 0.0f);  // lifetime
            vertices.push_back(v);
        }
        Engine::Mesh mesh;
		mesh.create(context, std::move(vertices));
        return mesh; // No indices for points
    }

    Texture ModelLoader::createTextureImage(VulkanContext& context, const char* filepath, bool srgb)
    {
        Texture texture{};
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels)
        {
            LOG(stbi_failure_reason());
            throw std::runtime_error("Failed to load texture image!");
        }
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
        VkFormat imageFormat = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { (uint32_t)texWidth, (uint32_t)texHeight, 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageFormat; // Use the determined format
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

        // Transition, copy, transition - use the same format
        transitionImageLayout(context, textureImage, imageFormat,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer.buffer, textureImage, (uint32_t)texWidth, (uint32_t)texHeight);

        transitionImageLayout(context, textureImage, imageFormat,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // staging no longer needed
        stagingBuffer.destroy(context.getDevice());

        // Create image view with matching format
        VkImageView textureImageView = createImageView(context, textureImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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


    std::tuple<VkSampler, std::array<Image, 6>> ModelLoader::LoadCubemapForSkybox(VulkanContext& context)
    {
        std::string path = "Objects/Cubemap.png";
        int srcW = 0, srcH = 0, srcChannels = 0;
        constexpr int reqChannels = STBI_rgb_alpha;
        stbi_uc* srcPixels = stbi_load(path.c_str(), &srcW, &srcH, &srcChannels, reqChannels);
        if (!srcPixels) {
            std::cerr << "Failed to load cubemap image: " << path << " - " << stbi_failure_reason() << std::endl;
            throw std::runtime_error("Failed to load cubemap image");
        }

        // Determine layout and face size.
        int faceW = 0, faceH = 0;
        enum class Layout { HorizontalStrip, VerticalStrip, Cross4x3 } layout = Layout::HorizontalStrip;

        if (srcW % 6 == 0 && srcH > 0 && (srcW / 6) == srcH) {
            // perfect horizontal strip (6x1)
            faceW = srcW / 6;
            faceH = srcH;
            layout = Layout::HorizontalStrip;
        }
        else if (srcH % 6 == 0 && srcW > 0 && (srcH / 6) == srcW) {
            // perfect vertical strip (1x6)
            faceW = srcW;
            faceH = srcH / 6;
            layout = Layout::VerticalStrip;
        }
        else if (srcW % 4 == 0 && srcH % 3 == 0 && (srcW / 4) == (srcH / 3)) {
            // 4x3 cross layout
            faceW = srcW / 4;
            faceH = srcH / 3;
            layout = Layout::Cross4x3;
        }
        else {
            stbi_image_free(srcPixels);
            std::cerr << "Unsupported cubemap layout for image: " << path << " (w=" << srcW << " h=" << srcH << ")" << std::endl;
            throw std::runtime_error("Unsupported cubemap layout");
        }

        const VkDeviceSize singleImageSize = static_cast<VkDeviceSize>(faceW) * faceH * 4;
        const size_t facesCount = 6;
        std::vector<uint8_t> combined;
        combined.resize(singleImageSize * facesCount);

        // Order required by the engine:
        // 0 +X Right
        // 1 -X Left
        // 2 +Y Top
        // 3 -Y Bottom
        // 4 +Z Front
        // 5 -Z Back
        // We'll compute source (x0,y0) for each face depending on layout.
        std::array<std::pair<int, int>, 6> srcOffsets{}; // {x0,y0} in pixels

        if (layout == Layout::HorizontalStrip) {
            for (int i = 0; i < 6; ++i) {
                srcOffsets[i] = { i * faceW, 0 };
            }
        }
        else if (layout == Layout::VerticalStrip) {
            for (int i = 0; i < 6; ++i) {
                srcOffsets[i] = { 0, i * faceH };
            }
        }
        else /* Cross4x3 */ {
            // Assume the common cross arrangement:
            // row0:   [   ][ +Y ][   ][   ]
            // row1:   [ -X ][ +Z ][ +X ][ -Z ]
            // row2:   [   ][ -Y ][   ][   ]
            //
            // Coordinates are (col, row) for each face:
            // +X -> (2,1)
            // -X -> (0,1)
            // +Y -> (1,0)
            // -Y -> (1,2)
            // +Z -> (1,1)
            // -Z -> (3,1)
            const std::array<std::pair<int, int>, 6> cell = {
                std::make_pair(2,1), // +X
                std::make_pair(0,1), // -X
                std::make_pair(1,0), // +Y
                std::make_pair(1,2), // -Y
                std::make_pair(1,1), // +Z
                std::make_pair(3,1)  // -Z
            };
            for (size_t i = 0; i < cell.size(); ++i) {
                srcOffsets[i] = { cell[i].first * faceW, cell[i].second * faceH };
            }
        }

        // Copy each face into 'combined' in the required order, row by row
        for (size_t f = 0; f < facesCount; ++f) {
            int sx = srcOffsets[f].first;
            int sy = srcOffsets[f].second;
            uint8_t* dstPtr = combined.data() + f * singleImageSize;
            for (int y = 0; y < faceH; ++y) {
                const stbi_uc* srcRow = srcPixels + ((sy + y) * srcW + sx) * 4;
                std::memcpy(dstPtr + static_cast<size_t>(y) * faceW * 4, srcRow, static_cast<size_t>(faceW) * 4);
            }
        }

        stbi_image_free(srcPixels);

        // Create staging buffer and upload combined data
        Engine::Buffer stagingBuffer;
        VkDeviceSize totalSize = singleImageSize * facesCount;
        stagingBuffer.create(context, totalSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        stagingBuffer.write(context.getDevice(), combined.data(), totalSize);

        // Create cube-compatible image (one VkImage, 6 array layers)
        VkImage cubeImage = VK_NULL_HANDLE;
        VkDeviceMemory cubeImageMemory = VK_NULL_HANDLE;
        VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { static_cast<uint32_t>(faceW), static_cast<uint32_t>(faceH), 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = static_cast<uint32_t>(facesCount); // 6
        imageInfo.format = imageFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        if (vkCreateImage(context.getDevice(), &imageInfo, nullptr, &cubeImage) != VK_SUCCESS)
            throw std::runtime_error("Failed to create cubemap image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(context.getDevice(), cubeImage, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = stagingBuffer.findMemoryType(context, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        if (vkAllocateMemory(context.getDevice(), &allocInfo, nullptr, &cubeImageMemory) != VK_SUCCESS)
            throw std::runtime_error("Failed to allocate cubemap image memory!");

        vkBindImageMemory(context.getDevice(), cubeImage, cubeImageMemory, 0);

        // Transition all layers UNDEFINED -> TRANSFER_DST_OPTIMAL
        {
            VkCommandBuffer cmd = beginSingleTimeCommands(context);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = cubeImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = static_cast<uint32_t>(facesCount);
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );
            endSingleTimeCommands(context, cmd);
        }

        // Copy each face into its layer
        {
            VkCommandBuffer cmd = beginSingleTimeCommands(context);
            std::vector<VkBufferImageCopy> regions(facesCount);
            for (size_t i = 0; i < facesCount; ++i)
            {
                VkBufferImageCopy region{};
                region.bufferOffset = singleImageSize * i;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = 0;
                region.imageSubresource.baseArrayLayer = static_cast<uint32_t>(i);
                region.imageSubresource.layerCount = 1;
                region.imageOffset = { 0, 0, 0 };
                region.imageExtent = { static_cast<uint32_t>(faceW), static_cast<uint32_t>(faceH), 1 };
                regions[i] = region;
            }

            vkCmdCopyBufferToImage(
                cmd,
                stagingBuffer.buffer,
                cubeImage,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                static_cast<uint32_t>(regions.size()),
                regions.data()
            );

            endSingleTimeCommands(context, cmd);
        }

        // Transition all layers -> SHADER_READ_ONLY_OPTIMAL
        {
            VkCommandBuffer cmd = beginSingleTimeCommands(context);
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = cubeImage;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = static_cast<uint32_t>(facesCount);
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            endSingleTimeCommands(context, cmd);
        }

        // staging no longer needed
        stagingBuffer.destroy(context.getDevice());
        combined.clear();

        // Create cube image view (VK_IMAGE_VIEW_TYPE_CUBE)
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubeImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = imageFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = static_cast<uint32_t>(facesCount);

        VkImageView cubeImageView;
        if (vkCreateImageView(context.getDevice(), &viewInfo, nullptr, &cubeImageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create cubemap image view!");

        // Create sampler
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler cubeSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &cubeSampler) != VK_SUCCESS)
            throw std::runtime_error("Failed to create cubemap sampler!");

        // Package minimal Image entries (all reference same image/view)
        std::array<Image, 6> result{};
        for (size_t i = 0; i < result.size(); ++i) {
            result[i].image = cubeImage;
            result[i].imageMemory = cubeImageMemory;
            result[i].imageView = cubeImageView;
        }

        return std::make_tuple(cubeSampler, result);
    }
}

