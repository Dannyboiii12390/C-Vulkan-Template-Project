#pragma once
#include <string>
#include "../Graphics/Mesh.h"
#include "../Graphics/VulkanTypes.h"
#include "../VulkanContext.h"
#include <array>
#include <tuple>	



namespace Engine {

	class ModelLoader
	{
	public:
		static Engine::Mesh createCube(VulkanContext& context);
		static Engine::Mesh createCubeWithoutIndex(VulkanContext& context);
		static Engine::Mesh createGrid(VulkanContext& context, int width, int depth);
		static Engine::Mesh createTerrain(VulkanContext& context, int width, int depth, float cellSize, int uvSize = 1);
		static Engine::Mesh createCylinder(VulkanContext& context, float radius, float height, int segmentCount, float UVsize = 1);
		static Engine::Mesh loadObj(VulkanContext& context, const char* filepath);
		static Engine::Mesh createSphere(VulkanContext& context, float radius, int sectorCount, int stackCount);
		static Engine::Mesh createParticleSystem(VulkanContext& context, int maxParticles);
        
		static Texture createTextureImage(VulkanContext& context, const char* filepath, bool srgb = true);
        static void transitionImageLayout(VulkanContext& context, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
        static void copyBufferToImage(VulkanContext& context, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        static VkCommandBuffer beginSingleTimeCommands(VulkanContext& context);
        static void endSingleTimeCommands(VulkanContext& context, VkCommandBuffer commandBuffer);
        static VkImageView createImageView(VulkanContext& context, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
		
		static VkSampler createTextureSampler(VulkanContext& context);
		static VkSampler createNearestSampler(VulkanContext& context);
		static VkSampler createBilinearSampler(VulkanContext& context);
		static VkSampler createTrilinearSampler(VulkanContext& context);
		static VkSampler createAnisotropicSampler(VulkanContext& context, float maxAnisotropy = 16.0f);

		static std::tuple<VkSampler, std::array<Image, 6>> LoadImagesForSkybox(VulkanContext& context);
	};
}


