#include "ModelLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <map>
#include <sstream>

#include "../VulkanContext.h"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace Engine
{
    const std::string ModelLoader::defaultFragmentShaderPath = "Shaders/textureFragLighting.frag.spv";
	const std::string ModelLoader::defaultVertexShaderPath = "Shaders/textureFragLighting.vert.spv";
    const std::string ModelLoader::globe = "GlobeGlass";

    void ModelLoader::applyTextureAndShaderConfig(VulkanContext& context, const std::string& shaderLine, const std::string& textureLine,Pipeline& pipeline, VkDescriptorSetLayout descriptorSetLayout, bool& hasTextures, Texture& albedoTex, Texture& normalTex)
    {
        std::string vertPath = defaultVertexShaderPath;
        std::string fragPath = defaultFragmentShaderPath;
        if (!shaderLine.empty())
        {
            std::istringstream sss(shaderLine);
            sss >> vertPath >> fragPath;
        }

        pipeline.create(
            context,
            vertPath,
            fragPath,
            context.getSwapchain().getImageFormat(),
            context.getSwapchain().getDepthFormat(),
            descriptorSetLayout, VK_CULL_MODE_NONE
        );

        if (!textureLine.empty())
        {
            std::istringstream tss(textureLine);
            std::string albedoPath, normalPath;
            tss >> albedoPath >> normalPath;
            if (!albedoPath.empty())
            {
                albedoTex = createTextureImage(context, albedoPath.c_str(), true);
                hasTextures = true;
            }
            if (!normalPath.empty())
            {
                normalTex = createTextureImage(context, normalPath.c_str(), false);
                hasTextures = true;
            }
        }

    }

    Mesh ModelLoader::createCube(const VulkanContext& context)
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
    Mesh ModelLoader::createCubeWithoutIndex(const VulkanContext& context)
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
    Mesh ModelLoader::createSphere(const VulkanContext& context,
        float radius, int sectorCount, int stackCount, float UVsize)
    {
        if (sectorCount < 3) sectorCount = 3;
        if (stackCount < 2) stackCount = 2; // at least two stacks to form a sphere
        if (radius <= 0.0f) radius = 1.0f;
        if (UVsize <= 0.0f) UVsize = 1.0f;

        const float PI = 3.14159265358979323846f;
        const float TWO_PI = glm::two_pi<float>();

        const int vertsPerRow = sectorCount + 1;
        const int rows = stackCount + 1;
        const size_t vertexCount = static_cast<size_t>(vertsPerRow) * static_cast<size_t>(rows);

        if (vertexCount > 65535u)
            throw std::runtime_error("Sphere: too many vertices for uint16_t indices");

        std::vector<Vertex> vertices;
        vertices.reserve(vertexCount);

        // Generate vertices
        for (int i = 0; i <= stackCount; ++i)
        {
            float t = static_cast<float>(i) / static_cast<float>(stackCount); // 0..1
            float theta = t * PI; // 0..PI
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            for (int j = 0; j <= sectorCount; ++j)
            {
                float s = static_cast<float>(j) / static_cast<float>(sectorCount); // 0..1
                float phi = s * TWO_PI; // 0..2PI
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                glm::vec3 pos = glm::vec3(
                    radius * sinTheta * cosPhi,
                    radius * cosTheta,
                    radius * sinTheta * sinPhi
                );

                glm::vec3 normal = glm::normalize(pos);

                // Tangent: direction of increasing phi (around Y)
                glm::vec3 tangent = glm::normalize(glm::vec3(-sinPhi, 0.0f, cosPhi));
                // Handle degeneracy (at poles tangent may be invalid)
                if (!std::isfinite(tangent.x) || glm::dot(tangent, tangent) < 1e-12f)
                {
                    tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                }

                glm::vec3 binormal = glm::normalize(glm::cross(normal, tangent));
                if (glm::dot(binormal, binormal) < 1e-12f)
                {
                    // fallback
                    binormal = glm::vec3(0.0f, 0.0f, 1.0f);
                }

                Vertex v{};
                v.pos = pos;
                v.normal = normal;
                v.tangent = tangent;
                v.binormal = binormal;
                v.color = glm::vec3(1.0f);
                v.texCoord = glm::vec2(s * UVsize, t * UVsize);

                vertices.push_back(v);
            }
        }

        // Build indices (two triangles per quad)
        std::vector<uint16_t> indices;
        indices.reserve(static_cast<size_t>(stackCount) * static_cast<size_t>(sectorCount) * 6);

        for (int i = 0; i < stackCount; ++i)
        {
            int k1 = i * (sectorCount + 1);       // beginning of current stack
            int k2 = k1 + sectorCount + 1;        // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // upper triangle (skip for top pole)
                if (i != 0)
                {
                    indices.push_back(static_cast<uint16_t>(k1));
                    indices.push_back(static_cast<uint16_t>(k2));
                    indices.push_back(static_cast<uint16_t>(k1 + 1));
                }

                // lower triangle (skip for bottom pole)
                if (i != (stackCount - 1))
                {
                    indices.push_back(static_cast<uint16_t>(k1 + 1));
                    indices.push_back(static_cast<uint16_t>(k2));
                    indices.push_back(static_cast<uint16_t>(k2 + 1));
                }
            }
        }

        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    }
    Mesh ModelLoader::createTorus(const VulkanContext& context,
        float outerRadius, float innerRadius, float height,
        int numSides, int numRings, float UVsize)
    {
        // ---- Parameter validation ----
        if (numSides < 3) numSides = 3;
        if (numRings < 3) numRings = 3;
        if (outerRadius <= 0.0f) outerRadius = 1.0f;
        if (innerRadius <= 0.0f) innerRadius = 0.25f;
        if (UVsize <= 0.0f) UVsize = 1.0f;
        if (height <= 0.0f) height = 1.0f; // height used as a vertical scale factor for the tube cross-section

        const int vertsPerRing = numSides + 1;
        const int ringCount = numRings + 1;
        const size_t vertexCount =
            static_cast<size_t>(vertsPerRing) * static_cast<size_t>(ringCount);

        if (vertexCount > 65535u)
            throw std::runtime_error("Torus: too many vertices for uint16_t indices");

        std::vector<Vertex> vertices;
        vertices.reserve(vertexCount);

        const float TWO_PI = glm::two_pi<float>();

        // Note on parameter semantics:
        // - outerRadius (R) is the major radius (distance from torus center to center of tube).
        // - innerRadius is treated as the tube (minor) radius (r). If you intended 'innerRadius'
        //   to be the hole radius (distance from torus center to inner surface), convert before
        //   calling: minorRadius = outerRadius - innerHoleRadius.
        //
        // - height acts as a vertical scale factor for the tube cross-section (1.0 = circular).
        //   This produces an elliptical cross-section when height != 1.0.

        const float R = outerRadius;
        const float r = innerRadius;
        const float h = height;

        // ---- Generate vertices ----
        for (int ring = 0; ring <= numRings; ++ring)
        {
            float u = float(ring) / float(numRings);   // major angle param
            float phi = u * TWO_PI;
            float cosPhi = std::cos(phi);
            float sinPhi = std::sin(phi);

            for (int side = 0; side <= numSides; ++side)
            {
                float v = float(side) / float(numSides);  // minor angle param
                float theta = v * TWO_PI;
                float cosTheta = std::cos(theta);
                float sinTheta = std::sin(theta);

                // ---- Position with optional vertical scaling (elliptical cross-section) ----
                glm::vec3 position(
                    (R + r * cosTheta) * cosPhi,
                    h * r * sinTheta,
                    (R + r * cosTheta) * sinPhi
                );

                // ---- Compute accurate normal via partial derivatives ----
                // p(φ,θ) = [ (R + r cosθ) cosφ, h r sinθ, (R + r cosθ) sinφ ]
                // ∂p/∂θ = [ -r sinθ cosφ, h r cosθ, -r sinθ sinφ ]
                // ∂p/∂φ = [ -(R + r cosθ) sinφ, 0, (R + r cosθ) cosφ ]
                glm::vec3 dp_dtheta = glm::vec3(-r * sinTheta * cosPhi, h * r * cosTheta, -r * sinTheta * sinPhi);
                glm::vec3 dp_dphi = glm::vec3(-(R + r * cosTheta) * sinPhi, 0.0f, (R + r * cosTheta) * cosPhi);

                glm::vec3 normal = glm::normalize(glm::cross(dp_dtheta, dp_dphi));

                // Tangent: direction of increasing φ (major direction)
                glm::vec3 tangent = glm::normalize(dp_dphi);

                // Binormal = N × T
                glm::vec3 binormal = glm::normalize(glm::cross(normal, tangent));

                Vertex vert{};
                vert.pos = position;
                vert.normal = normal;
                vert.tangent = tangent;
                vert.binormal = binormal;
                vert.color = glm::vec3(1.0f);
                vert.texCoord = glm::vec2(u * UVsize, v * UVsize);

                vertices.push_back(vert);
            }
        }

        // ---- Build indices (two triangles per quad) ----
        std::vector<uint16_t> indices;
        indices.reserve(size_t(numRings) * size_t(numSides) * 6u);

        for (int ring = 0; ring < numRings; ++ring)
        {
            int ringStart = ring * vertsPerRing;
            int nextRingStart = (ring + 1) * vertsPerRing;

            for (int side = 0; side < numSides; ++side)
            {
                uint16_t a = uint16_t(ringStart + side);
                uint16_t b = uint16_t(ringStart + side + 1);
                uint16_t c = uint16_t(nextRingStart + side);
                uint16_t d = uint16_t(nextRingStart + side + 1);

                // two triangles: (a, c, d) and (a, d, b)
                indices.push_back(a);
                indices.push_back(c);
                indices.push_back(d);

                indices.push_back(a);
                indices.push_back(d);
                indices.push_back(b);
            }
        }

        // ---- Upload to GPU ----
        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    }

    Mesh ModelLoader::createGrid(const VulkanContext& context, int width, int depth)
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
    Mesh ModelLoader::createQuad(const VulkanContext& context, float size, float UVsize)
    {
        // Create a single quad (two triangles) on the XY plane, facing +Z.
        // Vertex layout: pos, color, normal, texCoord, tangent, binormal
        if (size <= 0.0f) size = 1.0f;
        if (UVsize <= 0.0f) UVsize = 1.0f;

        const float half = 0.5f * size;

        glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec3 tangent = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 binormal = glm::vec3(0.0f, 1.0f, 0.0f);

        std::vector<Vertex> vertices;
        vertices.reserve(6);

        // Following same UV orientation as other primitives (bottom-left = (0,1))
        // Triangle 1: bottom-left, bottom-right, top-right
        {
            Vertex v{};
            v.pos = glm::vec3(-half, -half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(0.0f, 1.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }
        {
            Vertex v{};
            v.pos = glm::vec3(+half, -half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(1.0f, 1.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }
        {
            Vertex v{};
            v.pos = glm::vec3(+half, +half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(1.0f, 0.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }

        // Triangle 2: bottom-left, top-right, top-left
        {
            Vertex v{};
            v.pos = glm::vec3(-half, -half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(0.0f, 1.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }
        {
            Vertex v{};
            v.pos = glm::vec3(+half, +half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(1.0f, 0.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }
        {
            Vertex v{};
            v.pos = glm::vec3(-half, +half, 0.0f);
            v.color = glm::vec3(1.0f);
            v.normal = normal;
            v.texCoord = glm::vec2(0.0f, 0.0f) * UVsize;
            v.tangent = tangent;
            v.binormal = binormal;
            vertices.push_back(v);
        }

        Mesh mesh;
        // Non-indexed creation (matches other non-indexed primitives in this file)
        mesh.create(context, std::move(vertices));
        return mesh;
    }
    Mesh ModelLoader::createTerrain(const VulkanContext& context, int totalWidth, int totalDepth, float cellSize, float UVsize)
    {
        std::vector<Vertex> finalVertices;
        std::vector<uint16_t> indices; // use uint16_t to match Mesh::create signature

        // Validate inputs
        if (cellSize <= 0.0f) cellSize = 1.0f;
        if (totalWidth <= 0) totalWidth = 1;
        if (totalDepth <= 0) totalDepth = 1;

        // Number of cells
        int cellsX = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalWidth) / cellSize)));
        int cellsZ = std::max(1, static_cast<int>(std::floor(static_cast<float>(totalDepth) / cellSize)));

        const int vertsX = cellsX + 1;
        const int vertsZ = cellsZ + 1;

        const float halfWidth = (cellsX * cellSize) * 0.5f;
        const float halfDepth = (cellsZ * cellSize) * 0.5f;

        // Height function: flat terrain (height == 0)
        auto heightAt = [&](int ix, int iz) -> float
            {
                (void)ix; (void)iz; // avoid unused-parameter warnings
                return 0.0f;
            };

        auto positionAt = [&](int ix, int iz) -> glm::vec3
            {
                float wx = (static_cast<float>(ix) * cellSize) - halfWidth;
                float wz = (static_cast<float>(iz) * cellSize) - halfDepth;
                float y = heightAt(ix, iz);
                return glm::vec3(wx, y, wz);
            };

        // Build grid of positions and texcoords
        std::vector<glm::vec3> positions(vertsX * vertsZ);
        std::vector<glm::vec2> texcoords(vertsX * vertsZ);
        for (int iz = 0; iz < vertsZ; ++iz)
        {
            for (int ix = 0; ix < vertsX; ++ix)
            {
                int idx = iz * vertsX + ix;
                positions[idx] = positionAt(ix, iz);

                // Clamp radial magnitude: if position magnitude > 100, normalize and scale to 100
                float mag = glm::length(positions[idx]);
                if (mag > 100.0f)
                {
                    positions[idx] = glm::normalize(positions[idx]) * 100.0f;
                }

                texcoords[idx] = glm::vec2(static_cast<float>(ix) / static_cast<float>(cellsX),
                    static_cast<float>(iz) / static_cast<float>(cellsZ)) * UVsize;
            }
        }

        // Build indices (two triangles per quad)
        indices.reserve(static_cast<size_t>(cellsX) * static_cast<size_t>(cellsZ) * 6);
        for (int iz = 0; iz < cellsZ; ++iz)
        {
            for (int ix = 0; ix < cellsX; ++ix)
            {
                // compute indices and cast to uint16_t
                uint16_t a = static_cast<uint16_t>(iz * vertsX + ix);
                uint16_t b = static_cast<uint16_t>(iz * vertsX + (ix + 1));
                uint16_t c = static_cast<uint16_t>((iz + 1) * vertsX + ix);
                uint16_t d = static_cast<uint16_t>((iz + 1) * vertsX + (ix + 1));
                // First triangle: a, c, d
                indices.push_back(a);
                indices.push_back(c);
                indices.push_back(d);
                // Second triangle: a, d, b
                indices.push_back(a);
                indices.push_back(d);
                indices.push_back(b);
            }
        }

        // Compute per-vertex normals by accumulating face normals
        std::vector<glm::vec3> normals(positions.size(), glm::vec3(0.0f));
        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t ia = static_cast<uint32_t>(indices[i + 0]);
            uint32_t ib = static_cast<uint32_t>(indices[i + 1]);
            uint32_t ic = static_cast<uint32_t>(indices[i + 2]);
            glm::vec3 pa = positions[ia];
            glm::vec3 pb = positions[ib];
            glm::vec3 pc = positions[ic];
            glm::vec3 faceNormal = glm::normalize(glm::cross(pb - pa, pc - pa));
            normals[ia] += faceNormal;
            normals[ib] += faceNormal;
            normals[ic] += faceNormal;
        }
        for (size_t i = 0; i < normals.size(); ++i)
        {
            normals[i] = glm::normalize(normals[i]);
        }

        // Build final Vertex array (shared vertices)
        finalVertices.reserve(positions.size());
        for (size_t i = 0; i < positions.size(); ++i)
        {
            Vertex v{};
            v.pos = positions[i];
            v.normal = normals[i];
            v.texCoord = texcoords[i];
            v.color = glm::vec3(1.0f);
            // Provide a simple tangent/binormal; if you need accurate tangents, compute them explicitly.
            v.tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            v.binormal = glm::normalize(glm::cross(v.normal, v.tangent));
            finalVertices.push_back(v);
        }

        Mesh mesh;
        // Use indexed mesh creation (indices are uint16_t to match Mesh::create)
        mesh.create(context, std::move(finalVertices), std::move(indices));
        return mesh;
    }
    
    Mesh ModelLoader::loadOBJ(const VulkanContext& context, const char* filepath, float UVsize)
    {
        // Pseudocode:
        // 1. Open the file and read all lines.
        // 2. Parse vertex positions (v), texture coordinates (vt), normals (vn).
        // 3. Parse faces (f) and build indices.
        // 4. For each face, create Vertex objects (pos, normal, texCoord, color, tangent, binormal).
        // 5. Store vertices and indices, avoiding duplicates (vertex deduplication).
        // 6. Create Mesh using mesh.create(context, vertices, indices).

        std::ifstream file(filepath);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Failed to open OBJ file: ") + filepath);
        }

        // Keep Index and dedup map declared early — they are needed while parsing faces.
        std::map<Index, uint16_t> indexMap;
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        // Postpone allocation of heavy per-vertex attribute containers until just before parsing.
        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> texCoords;
        std::vector<glm::vec3> normals;

        std::string line;
        while (std::getline(file, line))
        {
            std::istringstream iss(line);
            std::string prefix;
            iss >> prefix;
            if (prefix == "v")
            {
                float x, y, z;
                iss >> x >> y >> z;
                positions.emplace_back(x, y, z);
            }
            else if (prefix == "vt")
            {
                float u, v;
                iss >> u >> v;
                texCoords.emplace_back(u * UVsize, v * UVsize);
            }
            else if (prefix == "vn")
            {
                float nx, ny, nz;
                iss >> nx >> ny >> nz;
                normals.emplace_back(nx, ny, nz);
            }
            else if (prefix == "f")
            {
                std::vector<Index> faceIndices;
                std::string vertStr;
                while (iss >> vertStr)
                {
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
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i)
                {
                    std::array<Index, 3> idxs { faceIndices[0], faceIndices[i], faceIndices[i + 1] };
                    for (int k = 0; k < 3; ++k)
                    {
                        auto it = indexMap.find(idxs[k]);
                        if (it == indexMap.end())
                        {
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
                        else
                        {
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

    Mesh ModelLoader::createSemiSphere(const VulkanContext& context, float radius, int sectorCount, int stackCount, float u)
    {
        // Produce a hollow hemisphere (open at the equator).
        if (sectorCount < 3) sectorCount = 3;
        if (stackCount < 1) stackCount = 1; // at least one stack to make a hemisphere
        if (radius <= 0.0f) radius = 1.0f;

        const float PI = 3.14159265358979323846f;
        std::vector<Vertex> vertices;
        std::vector<uint16_t> indices;

        vertices.reserve(static_cast<size_t>(stackCount + 1) * (sectorCount + 1));

        // Theta range restricted to [0 .. PI/2] to create a hemisphere (top -> equator)
        for (int i = 0; i <= stackCount; ++i)
        {
            float theta = 0.5 * PI * static_cast<float>(i) / static_cast<float>(stackCount); // 0..PI/2
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);

            for (int j = 0; j <= sectorCount; ++j)
            {
                float phi = 2.0f * PI * static_cast<float>(j) / static_cast<float>(sectorCount); // 0..2PI
                float sinPhi = std::sin(phi);
                float cosPhi = std::cos(phi);

                glm::vec3 pos = glm::vec3(radius * sinTheta * cosPhi,
                    radius * cosTheta,
                    radius * sinTheta * sinPhi);

                glm::vec3 normal = glm::normalize(pos);
                glm::vec3 tangent = glm::normalize(glm::vec3(-sinPhi, 0.0f, cosPhi));
                if (!std::isfinite(tangent.x) || lengthSq(tangent) < 1e-12f) tangent = glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 binormal = glm::normalize(glm::cross(normal, tangent));
                if (lengthSq(binormal) < 1e-12f) binormal = glm::vec3(0.0f, 0.0f, 1.0f);

                Vertex v{};
                v.pos = pos;
                v.normal = normal;
                v.color = glm::vec3(1.0f);
                v.texCoord = glm::vec2(u * static_cast<float>(j) / static_cast<float>(sectorCount),
                   u * static_cast<float>(i) / static_cast<float>(stackCount));
                v.tangent = tangent;
                v.binormal = binormal;

                vertices.push_back(v);
            }
        }

        // Build indices for the hemisphere surface only (no equator cap)
        for (int i = 0; i < stackCount; ++i)
        {
            int k1 = i * (sectorCount + 1);       // beginning of current stack
            int k2 = k1 + sectorCount + 1;        // beginning of next stack

            for (int j = 0; j < sectorCount; ++j, ++k1, ++k2)
            {
                // upper triangle (skip when at top stack==0)
                if (i != 0)
                {
                    indices.push_back(static_cast<uint16_t>(k1));
                    indices.push_back(static_cast<uint16_t>(k2));
                    indices.push_back(static_cast<uint16_t>(k1 + 1));
                }

                // lower triangle - for hemisphere we include this even for the stack just before equator
                indices.push_back(static_cast<uint16_t>(k1 + 1));
                indices.push_back(static_cast<uint16_t>(k2));
                indices.push_back(static_cast<uint16_t>(k2 + 1));
            }
        }

        // NOTE: equator cap removed to keep the dome hollow (open at equator).
        // If you want a thin-walled hemisphere with thickness, create an inner hemisphere (radius - thickness),
        // add flipped-wound indices for the inner surface, and connect the equator rings to form the rim.

        Mesh mesh;
        mesh.create(context, std::move(vertices), std::move(indices));
        return mesh;
    }

    Engine::Mesh ModelLoader::createParticleSystem(const VulkanContext& context, int particleCount, float area) {
        std::vector<Vertex> vertices;
        vertices.reserve(particleCount);

        // Create random particle positions
        for (uint32_t i = 0; i < particleCount; ++i) {
            Vertex v{};
            // Random position in a volume
            v.pos = glm::vec3(
                (rand() / (float)RAND_MAX - 0.5f) * area,
                (rand() / (float)RAND_MAX - 0.5f) * area,
                (rand() / (float)RAND_MAX - 0.5f) * area
            );
            v.normal = glm::normalize(v.pos);  // Use as velocity direction
            v.texCoord = glm::vec2(rand() / (float)RAND_MAX, 0.0f);  // lifetime
            vertices.push_back(v);
        }
        Engine::Mesh mesh;
		mesh.create(context, std::move(vertices));
        return mesh; // No indices for points
    }

    Texture ModelLoader::createTextureImage(const VulkanContext& context, const char* filepath, bool srgb)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        ASSERT(pixels, "Failed to load texture image!");

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
        transitionImageLayout(context, textureImage,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        copyBufferToImage(context, stagingBuffer.getBuffer(), textureImage, (uint32_t)texWidth, (uint32_t)texHeight);

        transitionImageLayout(context, textureImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        // staging no longer needed
        stagingBuffer.destroy(context.getDevice());

        // Create image view with matching format
        VkImageView textureImageView = createImageView(context, textureImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        VkSampler textureSampler = createTextureSampler(context);

        // IMPORTANT: store textureImage and textureImageMemory somewhere to free on cleanup (e.g., in VulkanContext or a resource manager)
        Texture texture{}; // postpone definition until just before use to reduce lifetime
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
    void ModelLoader::transitionImageLayout(const VulkanContext& context, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
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
    void ModelLoader::copyBufferToImage(const VulkanContext& context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
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
    VkCommandBuffer ModelLoader::beginSingleTimeCommands(const VulkanContext& context)
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
    void ModelLoader::endSingleTimeCommands(const VulkanContext& context, VkCommandBuffer commandBuffer)
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
    VkImageView ModelLoader::createImageView(const VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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

    VkSampler ModelLoader::createTextureSampler(const VulkanContext& context)
    {
        VkPhysicalDeviceFeatures deviceFeatures = getDeviceFeatures(context);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        ConfigureRepeatAddressModes(samplerInfo);
        samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        ConfigureCommonSamplerInfo(samplerInfo);

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;


        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createNearestSampler(const VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Nearest-neighbor filtering (no interpolation)
        samplerInfo.magFilter = VK_FILTER_NEAREST;
        samplerInfo.minFilter = VK_FILTER_NEAREST;

        ConfigureRepeatAddressModes(samplerInfo);

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
    VkSampler ModelLoader::createBilinearSampler(const VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Bilinear filtering (linear interpolation)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        ConfigureRepeatAddressModes(samplerInfo);

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
    VkSampler ModelLoader::createTrilinearSampler(const VulkanContext& context)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Trilinear filtering (linear + mipmap interpolation)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        ConfigureRepeatAddressModes(samplerInfo);

        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;

        ConfigureCommonSamplerInfo(samplerInfo);

        // Linear mipmap mode for trilinear
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 10.0f; // Allow multiple mipmap levels

        VkSampler textureSampler;
        if (vkCreateSampler(context.getDevice(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create trilinear sampler!");
        }

        return textureSampler;
    }
    VkSampler ModelLoader::createAnisotropicSampler(const VulkanContext& context, float maxAnisotropy)
    {
        VkPhysicalDeviceFeatures deviceFeatures{};
        vkGetPhysicalDeviceFeatures(context.getPhysicalDevice(), &deviceFeatures);

        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

        // Anisotropic filtering (best quality for oblique viewing angles)
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        ConfigureRepeatAddressModes(samplerInfo);

        // Enable anisotropic filtering if supported
        samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(context.getPhysicalDevice(), &properties);

		float maxAniso;
        if (deviceFeatures.samplerAnisotropy)
        {
            maxAniso = std::min(maxAnisotropy, properties.limits.maxSamplerAnisotropy);
        }
        else
        {
		 maxAniso = 1.0f;
        }

		samplerInfo.maxAnisotropy = maxAniso;

        // common fields
        ConfigureCommonSamplerInfo(samplerInfo);

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
    Texture ModelLoader::createTextureImageFromMemory(const VulkanContext& context, const void* pixelData, int texWidth, int texHeight, bool srgb)
    {
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(texWidth) * texHeight * 4;

        // staging
        Engine::Buffer stagingBuffer;
        stagingBuffer.create(context, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer.write(context.getDevice(), pixelData, imageSize);

        // create image
        VkImage textureImage = VK_NULL_HANDLE;
        VkDeviceMemory textureImageMemory = VK_NULL_HANDLE;
        VkFormat imageFormat = srgb ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM;
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = { static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 1 };
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageFormat;
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

        transitionImageLayout(context, textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(context, stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(context, textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        stagingBuffer.destroy(context.getDevice());

        VkImageView textureImageView = createImageView(context, textureImage, imageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        VkSampler textureSampler = createTextureSampler(context);

        Texture texture{};
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

    std::tuple<VkSampler, std::array<Image, 6>> ModelLoader::LoadCubemapForSkybox(const VulkanContext& context)
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
                stagingBuffer.getBuffer(),
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
    Engine::Texture ModelLoader::LoadCubemapForGlobe(const VulkanContext & context)
    {
        // Load the same source cubemap image used by LoadCubemapForSkybox
        std::string path = "Objects/Cubemap.png";
        int srcW = 0, srcH = 0, srcChannels = 0;
        constexpr int reqChannels = STBI_rgb_alpha;
        stbi_uc* srcPixels = stbi_load(path.c_str(), &srcW, &srcH, &srcChannels, reqChannels);
        if (!srcPixels) {
            std::cerr << "Failed to load cubemap image for globe: " << path << " - " << stbi_failure_reason() << std::endl;
            throw std::runtime_error("Failed to load cubemap image");
        }

        // Determine layout and face size (same logic as LoadCubemapForSkybox)
        int faceW = 0, faceH = 0;
        enum class Layout { HorizontalStrip, VerticalStrip, Cross4x3 } layout = Layout::HorizontalStrip;

        if (srcW % 6 == 0 && srcH > 0 && (srcW / 6) == srcH) {
            faceW = srcW / 6;
            faceH = srcH;
            layout = Layout::HorizontalStrip;
        }
        else if (srcH % 6 == 0 && srcW > 0 && (srcH / 6) == srcW) {
            faceW = srcW;
            faceH = srcH / 6;
            layout = Layout::VerticalStrip;
        }
        else if (srcW % 4 == 0 && srcH % 3 == 0 && (srcW / 4) == (srcH / 3)) {
            faceW = srcW / 4;
            faceH = srcH / 3;
            layout = Layout::Cross4x3;
        }
        else {
            stbi_image_free(srcPixels);
            std::cerr << "Unsupported cubemap layout for image: " << path << " (w=" << srcW << " h=" << srcH << ")" << std::endl;
            throw std::runtime_error("Unsupported cubemap layout");
        }

        // Compute offsets for each face in the source image using the same ordering as the engine.
        // Engine order: 0 +X, 1 -X, 2 +Y, 3 -Y, 4 +Z, 5 -Z
        std::array<std::pair<int, int>, 6> srcOffsets{};
        if (layout == Layout::HorizontalStrip) {
            for (int i = 0; i < 6; ++i) srcOffsets[i] = { i * faceW, 0 };
        }
        else if (layout == Layout::VerticalStrip) {
            for (int i = 0; i < 6; ++i) srcOffsets[i] = { 0, i * faceH };
        }
        else { // Cross4x3
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

        // Choose the 6th image (index 5, -Z/back face in engine ordering)
        const size_t faceIndex = 5;
        if (faceIndex >= srcOffsets.size()) {
            stbi_image_free(srcPixels);
            throw std::runtime_error("Requested face index out of range");
        }

        // Extract the selected face into a contiguous buffer (RGBA8)
        const VkDeviceSize faceSize = static_cast<VkDeviceSize>(faceW) * static_cast<VkDeviceSize>(faceH) * 4;
        std::vector<uint8_t> faceData;
        faceData.resize(faceSize);

        int sx = srcOffsets[faceIndex].first;
        int sy = srcOffsets[faceIndex].second;

        for (int y = 0; y < faceH; ++y) {
            const stbi_uc* srcRow = srcPixels + ((sy + y) * srcW + sx) * 4;
            uint8_t* dstRow = faceData.data() + static_cast<size_t>(y) * static_cast<size_t>(faceW) * 4;
            std::memcpy(dstRow, srcRow, static_cast<size_t>(faceW) * 4);
        }

        // Free the loaded source image now that we've extracted a face
        stbi_image_free(srcPixels);

        // Create a new 2D texture from the extracted face (SRGB)
        Engine::Texture tex = createTextureImageFromMemory(context, faceData.data(), faceW, faceH, true);

        // Return a self-owned 2D texture containing the chosen cubemap face.
        return tex;
    }
    void ModelLoader::loadFromConfig(
        VulkanContext& context,
        const std::string& configFilePath,
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorPool descriptorPool,
        std::vector<Buffer>& uniformBuffers,
        std::vector<Engine::Object>& objects,
        std::vector<Engine::ParticleSystem>& particleSystems,
        glm::vec3& globeCenter,
        float& globeRadius,
        Camera& C1,
        Camera& C2,
        Camera& C3,
        ParticleManager& particleManager,
        glm::vec3& sunDirection,
        glm::vec3& moonDirection
    )
    {
        std::ifstream file(configFilePath);
        if (!file.is_open())
        {
            throw std::runtime_error(std::string("Failed to open config file: ") + configFilePath);
        }

        // helper to trim
        auto trim = [](std::string& s)
            {
                const char* ws = " \t\r\n";
                size_t b = s.find_first_not_of(ws);
                size_t e = s.find_last_not_of(ws);
                if (b == std::string::npos) { s.clear(); return; }
                s = s.substr(b, e - b + 1);
            };

        // pending line support (when we read a line that belongs to next entry)
        std::string pendingLine;

        auto readNextLine = [&](std::string& out) -> bool
            {
                if (!pendingLine.empty())
                {
                    out = std::move(pendingLine);
                    pendingLine.clear();
                    return true;
                }
                while (std::getline(file, out))
                {
                    trim(out);
                    if (out.empty()) continue;
                    // skip full-line comments
                    if (!out.empty() && out[0] == '#') continue;
                    return true;
                }
                return false;
            };

        auto peekFirstToken = [](const std::string& line) -> std::string
            {
                std::istringstream ss(line);
                std::string t;
                ss >> t;
                return t;
            };

        // Ensure output containers are empty before filling
        objects.clear();
        particleSystems.clear();

        std::string line;
		const std::string pos = "pos";
        while (readNextLine(line))
        {
            std::istringstream iss(line);
            std::string token;
            iss >> token;
            if (token == "o")
            {
                // object entry
                std::string objName;
                if (!(iss >> objName))
                {
                    continue;
                }

                // read next data line(s) as needed
                std::string paramLine, shaderLine, textureLine, transformLine;
                // Read up to 3 non-header lines after the 'o <name>' header: param, shader, texture (all optional).
                // Stop early if we encounter a transform line (pos/rot/scale).
                for (int i = 0; i < 3; ++i)
                {
                    std::string nextLine;
                    if (!readNextLine(nextLine)) break;
                    std::string firstTok = peekFirstToken(nextLine);
                    // Check for section headers
                    if (firstTok == "o" || firstTok == "p" || firstTok == "c" || firstTok == "season" || firstTok == "light" || firstTok == "snow_probability" || firstTok == "snow_increment" || firstTok == "ls")
                    {
                        pendingLine = std::move(nextLine);
                        break;
                    }
                    // Check for transform keywords - these are NOT data lines
                    if (firstTok == pos || firstTok == "rot" || firstTok == "scale")
                    {
                        transformLine = std::move(nextLine);
                        break;
                    }
                    // Assign to appropriate data line slot
                    if (i == 0) paramLine = std::move(nextLine);
                    else if (i == 1) shaderLine = std::move(nextLine);
                    else if (i == 2) textureLine = std::move(nextLine);
                }

                // If we didn't find a transform line yet, try to read one more line
                if (transformLine.empty())
                {
                    std::string maybeTransform;
                    if (readNextLine(maybeTransform))
                    {
                        std::string ft = peekFirstToken(maybeTransform);
                        if (ft == pos || ft == "rot" || ft == "scale")
                        {
                            transformLine = std::move(maybeTransform);
                        }
                        else
                        {
                            pendingLine = std::move(maybeTransform);
                        }
                    }
                }

                try
                {
                    Engine::Mesh mesh;
                    bool hasTextures = false;
                    Engine::Texture albedoTex{}, normalTex{};
                    Engine::Pipeline pipeline;

                    // Handle different object types
                    if (objName == "terrain")
                    {
                        float uvsize = 1.0f;
                        if (!paramLine.empty())
                        {
                            std::istringstream pss(paramLine);
                            float cellSize = 1.0f;
                            float radius = 1.0f;
                            float height = 1.0f;
                            int segmentCount = 1;
                            pss >> radius >> height >> segmentCount >> uvsize;
                        }

                        mesh = createTerrain(context, 200, 200, 1, uvsize);

                        applyTextureAndShaderConfig(context, shaderLine, textureLine,
                            pipeline, descriptorSetLayout, hasTextures,
                            albedoTex, normalTex);
                    }
                    else if (objName == globe)
                    {
                        // Simplified Globe handling - single pipeline, standard rendering
                        float meshRadius = 100.0f;
                        int sectors = 64, stacks = 32;
                        if (!paramLine.empty())
                        {
                            std::istringstream pss(paramLine);
                            pss >> meshRadius >> sectors >> stacks;
                        }

                        mesh = createSemiSphere(context, meshRadius, sectors, stacks, 1.0f);

                        // Parse textures
                        if (!textureLine.empty())
                        {
                            std::istringstream tss(textureLine);
                            std::string albedoPath, normalPath;
                            tss >> albedoPath >> normalPath;
                            if (!albedoPath.empty())
                            {
                                albedoTex = createTextureImage(context, albedoPath.c_str(), true);
                                hasTextures = true;
                            }
                            if (!normalPath.empty())
                            {
                                normalTex = createTextureImage(context, normalPath.c_str(), false);
                                hasTextures = true;
                            }
                        }

                        // Create fallback textures if none provided
                        if (!hasTextures)
                        {
                            uint8_t whitePixel[4] = { 255, 255, 255, 255 };
                            uint8_t normalPixel[4] = { 128, 128, 255, 255 };
                            albedoTex = createTextureImageFromMemory(context, whitePixel, 1, 1, true);
                            normalTex = createTextureImageFromMemory(context, normalPixel, 1, 1, false);
                            hasTextures = true;
                        }

                        // Use standard shaders or custom ones
                        std::string vertPath = "shaders/globe.vert.spv";
                        std::string fragPath = "shaders/globe.frag.spv";
                        if (!shaderLine.empty())
                        {
                            std::istringstream sss(shaderLine);
                            sss >> vertPath >> fragPath;
                        }

                        pipeline.create(context, vertPath, fragPath,
                            context.getSwapchain().getImageFormat(),
                            context.getSwapchain().getDepthFormat(),
                            descriptorSetLayout,
                            VK_CULL_MODE_BACK_BIT,
                            true);

                        // Store globe info
                        globeCenter = glm::vec3(0.0f);
                        globeRadius = meshRadius;
                    }
                    else if (objName == "obj" || objName == "cactus" || objName == "rock" || objName == "grotto" || objName == "Grotto")
                    {
                        std::string objPath;
                        float uvsize = 1.0f;
                        if (!paramLine.empty())
                        {
                            std::istringstream pss(paramLine);
                            pss >> objPath >> uvsize;
                        }
                        if (objPath.empty()) continue;

                        mesh = loadOBJ(context, objPath.c_str(), uvsize);

                        applyTextureAndShaderConfig(context, shaderLine, textureLine,
                            pipeline, descriptorSetLayout, hasTextures,
                            albedoTex, normalTex);
                    }
                    else if (objName == "Torus" || objName == "GlobeGlass")
                    {
                        if (objName == "Torus")
                        {
                            float outerR = 3.0f, innerR = 0.5f, height = 1.0f;
                            int sides = 24, rings = 32;
                            float uvsize = 1.0f;
                            if (!paramLine.empty())
                            {
                                std::istringstream pss(paramLine);
                                pss >> outerR >> innerR >> height >> sides >> rings >> uvsize;
                            }
                            mesh = createTorus(context, outerR, innerR, height, sides, rings, uvsize);

                            if (!textureLine.empty())
                            {
                                std::istringstream tss(textureLine);
                                std::string albedoPath, normalPath;
                                tss >> albedoPath >> normalPath;
                                if (!albedoPath.empty())
                                {
                                    albedoTex = createTextureImage(context, albedoPath.c_str(), true);
                                    hasTextures = true;
                                }
                                if (!normalPath.empty())
                                {
                                    normalTex = createTextureImage(context, normalPath.c_str(), false);
                                    hasTextures = true;
                                }
                            }
                            if (!hasTextures)
                            {
                                uint8_t whitePixel[4] = { 255, 255, 255, 255 };
                                uint8_t normalPixel[4] = { 128, 128, 255, 255 };
                                albedoTex = createTextureImageFromMemory(context, whitePixel, 1, 1, true);
                                normalTex = createTextureImageFromMemory(context, normalPixel, 1, 1, false);
                                hasTextures = true;
                            }
                        }
                        else
                        {
                            float radius = 5.0f;
                            int sectors = 32, stacks = 16;
                            if (!paramLine.empty())
                            {
                                std::istringstream pss(paramLine);
                                pss >> radius >> sectors >> stacks;
                            }
                            mesh = createSemiSphere(context, radius, sectors, stacks);
                            hasTextures = false;
                        }

                        std::string vertPath = defaultVertexShaderPath;
                        std::string fragPath = defaultFragmentShaderPath;
                        if (!shaderLine.empty())
                        {
                            std::istringstream sss(shaderLine);
                            sss >> vertPath >> fragPath;
                        }

                        pipeline.create(context, vertPath, fragPath,
                            context.getSwapchain().getImageFormat(),
                            context.getSwapchain().getDepthFormat(),
                            descriptorSetLayout);
                    }
                    else
                    {
                        std::string firstToken;
                        if (!paramLine.empty())
                        {
                            std::istringstream pss(paramLine);
                            pss >> firstToken;
                        }
                        if (!firstToken.empty() && (firstToken.find(".obj") != std::string::npos || firstToken.find(".OBJ") != std::string::npos))
                        {
                            std::string objPath = firstToken;
                            float uvsize = 1.0f;
                            {
                                std::istringstream pss(paramLine);
                                pss >> objPath >> uvsize;
                            }
                            mesh = loadOBJ(context, objPath.c_str(), uvsize);
                        }
                        else
                        {
                            continue;
                        }

                        std::string vertPath = defaultVertexShaderPath;
                        std::string fragPath = defaultFragmentShaderPath;
                        if (!shaderLine.empty())
                        {
                            std::istringstream sss(shaderLine);
                            sss >> vertPath >> fragPath;
                        }

                        if (!textureLine.empty())
                        {
                            std::istringstream tss(textureLine);
                            std::string albedoPath, normalPath;
                            tss >> albedoPath >> normalPath;
                            if (!albedoPath.empty())
                            {
                                albedoTex = createTextureImage(context, albedoPath.c_str(), true);
                                hasTextures = true;
                            }
                            if (!normalPath.empty())
                            {
                                normalTex = createTextureImage(context, normalPath.c_str(), false);
                                hasTextures = true;
                            }
                        }

                        pipeline.create(context, vertPath, fragPath,
                            context.getSwapchain().getImageFormat(),
                            context.getSwapchain().getDepthFormat(),
                            descriptorSetLayout);
                    }

                    // Build Engine::Object and create with appropriate overload
                    Engine::Object obj;
                    if (hasTextures)
                    {
                        obj.create(context, std::move(mesh), std::move(pipeline), descriptorSetLayout, descriptorPool, uniformBuffers, albedoTex, normalTex);
                    }
                    else
                    {
                        obj.create(context, std::move(mesh), std::move(pipeline), descriptorSetLayout, descriptorPool, uniformBuffers);
                    }

                    obj.setName(objName);

                    // Parse transformLine (pos/rot/scale may appear in any order)
                    if (!transformLine.empty())
                    {
                        std::istringstream tss(transformLine);
                        std::string tkn;
                        while (tss >> tkn)
                        {
                            if (tkn == pos)
                            {
                                float x = 0, y = 0, z = 0; tss >> x >> y >> z;
                                obj.setPosition(glm::vec3(x, y, z));
                            }
                            else if (tkn == "rot")
                            {
                                float rx = 0, ry = 0, rz = 0; tss >> rx >> ry >> rz;
                                obj.setRotation(glm::vec3(rx, ry, rz));
                            }
                            else if (tkn == "scale")
                            {
                                float sx = 1, sy = 1, sz = 1; tss >> sx >> sy >> sz;
                                obj.setScale(glm::vec3(sx, sy, sz));
                            }
                        }
                    }

                    // Bind cubemap for GlobeGlass objects
                    if (objName == globe)
                    {
                        VkImageView cubemapView = context.getCubemapView();
                        VkSampler cubemapSampler = context.getCubemapSampler();
                        obj.updateCubemapBinding(context, cubemapView, cubemapSampler);
                    }

                    objects.push_back(std::move(obj));

                    // Destroy temporary Texture objects created for this entry
                    if (hasTextures)
                    {
                        albedoTex.destroy(context.getDevice());
                        normalTex.destroy(context.getDevice());
                    }
                }
                catch (...)
                {
                    throw;
                }
            }
            else if (token == "p")
            {
                // particle system
                std::string psName;
                iss >> psName;
                std::string countLine, shaderLine, transformLine;
                if (!readNextLine(countLine)) countLine.clear();
                if (!readNextLine(shaderLine)) shaderLine.clear();
                if (!readNextLine(transformLine)) transformLine.clear();

                if (!transformLine.empty())
                {
                    std::string ft = peekFirstToken(transformLine);
                    if (ft == "o" || ft == "p" || ft == "c")
                    {
                        pendingLine = std::move(transformLine);
                        transformLine.clear();
                    }
                }

                int particleCount = 1000;
                float area = 2.0f;
                if (!countLine.empty())
                {
                    std::istringstream css(countLine);
                    css >> particleCount >> area;
                }

                std::string vertPath = "shaders/particle.vert.spv";
                std::string fragPath = "shaders/particle.frag.spv";
                if (!shaderLine.empty())
                {
                    std::istringstream sss(shaderLine);
                    sss >> vertPath >> fragPath;
                }

                Engine::Mesh pm = createParticleSystem(context, particleCount, area);

                Engine::ParticlePipeline particlePipeline;
                particlePipeline.create(context, vertPath, fragPath,
                    context.getSwapchain().getImageFormat(),
                    context.getSwapchain().getDepthFormat(),
                    descriptorSetLayout);

                Engine::ParticleSystem ps;
                ps.create(context, std::move(pm), std::move(particlePipeline), descriptorSetLayout, descriptorPool, uniformBuffers);
                ps.setName(psName);
                ps.setActive(false);

                if (!transformLine.empty())
                {
                    std::istringstream tss(transformLine);
                    std::string tkn;
                    while (tss >> tkn)
                    {
                        if (tkn == pos)
                        {
                            float x = 0, y = 0, z = 0; tss >> x >> y >> z;
                            ps.setPosition(glm::vec3(x, y, z));
                        }
                        else if (tkn == "scale")
                        {
                            float sx = 1, sy = 1, sz = 1; tss >> sx >> sy >> sz;
                            ps.setScale(glm::vec3(sx, sy, sz));
                        }
                    }
                }

                particleSystems.push_back(std::move(ps));
            }
            else if (token == "c")
            {
                // camera entry
                std::string camName;
                iss >> camName;

                Camera* target = nullptr;
                if (camName == "C1" || camName == "c1") target = &C1;
                else if (camName == "C2" || camName == "c2") target = &C2;
                else if (camName == "C3" || camName == "c3") target = &C3;

                std::string nextLine;
                while (readNextLine(nextLine))
                {
                    std::string ft = peekFirstToken(nextLine);

                    if (ft == pos)
                    {
                        std::istringstream pss(nextLine);
                        std::string tok; pss >> tok;
                        float x = 0.0f, y = 0.0f, z = 0.0f;
                        pss >> x >> y >> z;
                        if (target) target->setPosition(glm::vec3(x, y, z));
                        continue;
                    }
                    else if (ft == "look")
                    {
                        std::istringstream lss(nextLine);
                        std::string tok; lss >> tok;
                        float lx = 0.0f, ly = 0.0f, lz = 0.0f;
                        lss >> lx >> ly >> lz;
                        if (target) target->lookAt(glm::vec3(lx, ly, lz));
                        continue;
                    }
                    else if (ft == "rot")
                    {
                        std::istringstream rss(nextLine);
                        std::string tok; rss >> tok;
                        float rx = 0.0f, ry = 0.0f, rz = 0.0f;
                        rss >> rx >> ry >> rz;
                        if (target) target->rotate(rx, ry);
                        continue;
                    }
                    else
                    {
                        pendingLine = std::move(nextLine);
                        break;
                    }
                }
                continue;
            }
            else if (token == "season" || token == "s")
            {
                std::string seasonName;
                iss >> seasonName;
                if (seasonName.empty()) continue;

                std::string sname = seasonName;
                for (auto& ch : sname) ch = static_cast<char>(std::tolower(ch));

                float snowProb = -1.0f;
                float sunshineThresh = -1.0f;
                float chanceInc = -1.0f;

                if (!(iss >> snowProb >> sunshineThresh >> chanceInc))
                {
                    if (sname == "summer")
                    {
                        snowProb = 0.0f;
                        sunshineThresh = 3.0f;
                        chanceInc = 0.3f;
                    }
                    else if (sname == "winter")
                    {
                        snowProb = 0.9f;
                        sunshineThresh = 15.0f;
                        chanceInc = 0.05f;
                    }
                    else if (sname == "spring")
                    {
                        snowProb = 0.2f;
                        sunshineThresh = 5.0f;
                        chanceInc = 0.15f;
                    }
                    else if (sname == "autumn" || sname == "fall")
                    {
                        snowProb = 0.4f;
                        sunshineThresh = 7.0f;
                        chanceInc = 0.1f;
                    }
                    else continue;
                }

                particleManager.setSnowProbability(glm::clamp(snowProb, 0.0f, 1.0f));
                particleManager.setSunshineThreshold(sunshineThresh > 0.0f ? sunshineThresh : 5.0f);
                particleManager.setChanceIncrement(chanceInc > 0.0f ? chanceInc : 0.2f);
                continue;
            }
            else if (token == "light" || token == "l" || token == "ls")
            {
                std::string which;
                if (token == "ls")
                {
                    iss >> which;
                    std::string nextLine;
                    if (!readNextLine(nextLine)) continue;
                    std::istringstream nds(nextLine);
                    std::string tok; nds >> tok;
                    if (tok != "dir")
                    {
                        pendingLine = std::move(nextLine);
                        continue;
                    }
                    float x = 0.0f, y = 0.0f, z = 0.0f;
                    nds >> x >> y >> z;
                    glm::vec3 dir(x, y, z);
                    if (glm::length(dir) > 1e-6f) dir = glm::normalize(dir);
                    const float LIGHT_DISTANCE = 200.0f; // same scale as VulkanContext::orbitRadius
                    glm::vec3 lightPos = dir * LIGHT_DISTANCE;
                    std::string w = which;
                    for (auto& ch : w) ch = static_cast<char>(std::tolower(ch));
                    if (w == "sun") sunDirection = lightPos;
                    else if (w == "moon") moonDirection = lightPos;
                    continue;
                }
                else
                {
                    iss >> which;
                    if (which.empty()) continue;
                    float x = 0.0f, y = 0.0f, z = 0.0f;
                    if (!(iss >> x >> y >> z))
                    {
                        std::string nextLine;
                        if (readNextLine(nextLine))
                        {
                            std::istringstream nss(nextLine);
                            std::string ft = peekFirstToken(nextLine);
                            if (ft == "dir")
                            {
                                std::string tok; nss >> tok;
                                nss >> x >> y >> z;
                            }
                            else
                            {
                                pendingLine = std::move(nextLine);
                                continue;
                            }
                        }
                        else continue;
                    }

                    glm::vec3 dir(x, y, z);
                    if (glm::length(dir) > 1e-6f) dir = glm::normalize(dir);
                    const float LIGHT_DISTANCE = 200.0f;
                    glm::vec3 lightPos = dir * LIGHT_DISTANCE;
                    std::string w = which;
                    for (auto& ch : w) ch = static_cast<char>(std::tolower(ch));
                    if (w == "sun") sunDirection = lightPos;
                    else if (w == "moon") moonDirection = lightPos;
                    continue;
                }
            }
            else if (token == "snow_probability")
            {
                float p = 0.0f;
                if (iss >> p)
                {
                    particleManager.setSnowProbability(glm::clamp(p, 0.0f, 1.0f));
                }
                else
                {
                    std::string next;
                    if (readNextLine(next))
                    {
                        std::istringstream nss(next);
                        float np;
                        if (nss >> np)
                        {
                            particleManager.setSnowProbability(glm::clamp(np, 0.0f, 1.0f));
                        }
                        else
                        {
                            pendingLine = std::move(next);
                        }
                    }
                }
                continue;
            }
            else if (token == "snow_increment")
            {
                float inc = 0.0f;
                if (iss >> inc)
                {
                    particleManager.setChanceIncrement(inc);
                }
                else
                {
                    std::string next;
                    if (readNextLine(next))
                    {
                        std::istringstream nss(next);
                        float nv;
                        if (nss >> nv)
                        {
                            particleManager.setChanceIncrement(nv);
                        }
                        else
                        {
                            pendingLine = std::move(next);
                        }
                    }
                }
                continue;
            }
        }
    }

    void ModelLoader::reloadFromConfig(
        VulkanContext& context,
        const std::string& configFilePath,
        VkDescriptorSetLayout descriptorSetLayout,
        VkDescriptorPool descriptorPool,
        std::vector<Buffer>& uniformBuffers,
        std::vector<Engine::Object>& objects,
		std::vector<Engine::ParticleSystem>& particleSystems,
        glm::vec3& globeCenter,
        float& globeRadius,
        Camera& C1,
        Camera& C2,
        Camera& C3,
        ParticleManager& particleManager,
        glm::vec3& sunDirection,
        glm::vec3& moonDirection
    ) {

        vkDeviceWaitIdle(context.getDevice());
		std::cout << "Reloading models from config" << std::endl;

        // Cleanup existing objects and free GPU resources
        for (auto& obj : objects)
        {
            obj.cleanup(context);
        }
        objects.clear();
        for (auto& ps : particleSystems)
        {
            ps.cleanup(context);
        }
        particleSystems.clear();

        // avoids copying data. resulting in undestroyed resources.
        Engine::ModelLoader::loadFromConfig(context, configFilePath, descriptorSetLayout, descriptorPool, uniformBuffers, objects, particleSystems, 
                globeCenter, globeRadius, C1, C2, C3, particleManager, sunDirection, moonDirection);

        // CRITICAL FIX: Update shadow map bindings for newly loaded objects
        context.updateObjectDescriptorSetsWithShadowMap();
    }
}


