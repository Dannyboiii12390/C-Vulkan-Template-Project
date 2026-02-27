#pragma once
#include <string>
#include "../Graphics/Mesh.h"
#include "../Graphics/VulkanTypes.h"
#include "../VulkanContext.h"
#include <array>
#include <tuple>	



namespace Engine {

	struct Index 
	{ 
		int v, vt, vn; 
		bool operator<(const Index& o) const { return std::tie(v, vt, vn) < std::tie(o.v, o.vt, o.vn); } 
	};

	inline float lengthSq(const glm::vec3& v) noexcept { return glm::dot(v, v); }

	inline VkPhysicalDeviceFeatures getDeviceFeatures(const VulkanContext& context)
	{
		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceFeatures(context.getPhysicalDevice(), &deviceFeatures);
		return deviceFeatures;
	}
	// Consolidate duplicated sampler initialization to remove repeated code (CDD.DUPC)
	inline void ConfigureCommonSamplerInfo(VkSamplerCreateInfo& samplerInfo)
	{
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipLodBias = 0.0f;
	}
	static inline void ConfigureRepeatAddressModes(VkSamplerCreateInfo& samplerInfo)
	{
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	class ModelLoader final
	{
	public:
		static void applyTextureAndShaderConfig(VulkanContext& context, const std::string& shaderLine, const std::string& textureLine, Pipeline& pipeline, VkDescriptorSetLayout descriptorSetLayout, bool& hasTextures, Texture& albedoTex, Texture& normalTex);
		static const std::string defaultFragmentShaderPath;
		static const std::string defaultVertexShaderPath;
		static const std::string globe;
		static Engine::Mesh createCube(const VulkanContext& context);
		static Engine::Mesh createCubeWithoutIndex(const VulkanContext& context);
		static Engine::Mesh createTorus(const VulkanContext& context, float outerRadius, float innerRadius, float height, int numSides, int numRings, float UVsize = 1);
		static Engine::Mesh createGrid(const VulkanContext& context, int width, int depth);
		static Engine::Mesh createTerrain(const VulkanContext& context, int width, int depth, float cellSize, float UVsize = 1);
		static Engine::Mesh createQuad(const VulkanContext& context, float size = 1.0f, float UVsize = 1);
		static Engine::Mesh createCylinder(const VulkanContext& context, float radius, float height, int segmentCount, float UVsize = 1);
		static Engine::Mesh createSphere(const VulkanContext& context, float radius, int sectorCount, int stackCount, float UVsize = 1);
		static Engine::Mesh loadOBJ(const VulkanContext& context, const const char* filepath, float UVsize = 1);

		static Engine::Mesh createSemiSphere(const VulkanContext& context, float radius, int sectorCount, int stackCount, float uvSize = 1.0f);
		static Engine::Mesh createParticleSystem(const VulkanContext& context, int maxParticles, float area = 2.0f);
        
		static Texture createTextureImage(const VulkanContext& context, const char* filepath, bool srgb = true);
        static void transitionImageLayout(const VulkanContext& context, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
        static void copyBufferToImage(const VulkanContext& context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static VkCommandBuffer beginSingleTimeCommands(const VulkanContext& context);
        static void endSingleTimeCommands(const VulkanContext& context, VkCommandBuffer commandBuffer);
        static VkImageView createImageView(const VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		
		static VkSampler createTextureSampler(const VulkanContext& context);
		static VkSampler createNearestSampler(const VulkanContext& context);
		static VkSampler createBilinearSampler(const VulkanContext& context);
		static VkSampler createTrilinearSampler(const VulkanContext& context);
		static VkSampler createAnisotropicSampler(const VulkanContext& context, float maxAnisotropy = 16.0f);
		static Texture createTextureImageFromMemory(const VulkanContext& context, const void* pixelData, int texWidth, int texHeight, bool srgb);

		static std::tuple<VkSampler, std::array<Image, 6>> LoadCubemapForSkybox(const VulkanContext& context);
		static Engine::Texture LoadCubemapForGlobe(const VulkanContext& context);
		static void loadFromConfig(
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
		);
		static void reloadFromConfig(
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
		);
	};
}


