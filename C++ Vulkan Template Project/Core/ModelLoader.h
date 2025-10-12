#pragma once
#include <string>
#include "../Graphics/Mesh.h"

namespace Engine {
	class ModelLoader
	{
	public:
		static Engine::Mesh loadCube(VulkanContext& context);
		static Engine::Mesh createGrid(VulkanContext& context, int width, int depth);
		static Engine::Mesh createTerrain(VulkanContext& context, int width, int depth, float cellSize);
		static Engine::Mesh createCylinder(VulkanContext& context, float radius, float height, int segmentCount);
		static Engine::Mesh loadObj(VulkanContext& context, const char* filepath);
	};
}


