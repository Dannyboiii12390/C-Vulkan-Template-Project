#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

#include "../Graphics/ParticleSystem.h"
#include "../Graphics/Object.h"
#include "../Graphics/VulkanTypes.h"
namespace Engine
{
    class ParticleManager final
    {
    public:
        ParticleManager() = default;
        ~ParticleManager() = default;

        // Non-copyable
        ParticleManager(const ParticleManager&) = delete;
        ParticleManager& operator=(const ParticleManager&) = delete;

        // Initialize manager with pointers to the engine containers (must remain valid)
        void init(std::vector<Engine::ParticleSystem>* particleSystems,
            std::vector<Engine::Object>* objects);

        // Per-frame update: deltaTime in seconds (use same time scale you use for particles) and the latest UBO
        void update(float deltaTime, const Engine::UniformBufferObject& ubo);

        // Tunables
        void setSunshineThreshold(float seconds) { sunshineThreshold = seconds; }
        void setCactusGrowthMultiplier(float m) { cactusGrowthMultiplier = m; }
        void setCactusGrowthDuration(float seconds) { defaultGrowthDuration = seconds; }
		void setChanceIncrement(float incrementPerSecond) { chanceIncrement = incrementPerSecond; }

        // Chance that a rain event becomes snow instead (0.0 - 1.0)
        void setSnowProbability(float p) { snowProbability = glm::clamp(p, 0.0f, 1.0f); }

		float getSnowProbability() const { return snowProbability; }

    private:
        // Non-owning pointers to scene lists
        std::vector<Engine::ParticleSystem>* particleSystems = nullptr;
        std::vector<Engine::Object>* objects = nullptr;

        // internal state
        float sunshineAccumulator = 0.0f;
        float sunshineThreshold = 5.0f; // default seconds required to ignite
        float cactusGrowthMultiplier = 1.05f;
        float defaultGrowthDuration = 1.0f;

        // Probability [0..1] that a rain start becomes snow instead
        float snowProbability = 0.1f;
		float chanceAccumulator = 0.0f;
		float chanceIncrement = 0.2f; // how much to increase chance accumulator per second of sunshine

        bool rainWasActive = false;
        bool snowWasActive = false;
        bool fireActive = false;

        // If precipitation ended but sun wasn't shining at that moment, wait until sun appears
        bool precipEndedWaitingForSun = false;

        struct CactusGrowth
        {
			std::size_t objIndex = 0;
            Engine::Object* obj = nullptr;
            glm::vec3 startScale{ 1.0f };
            glm::vec3 targetScale{ 1.0f };
            float elapsed = 0.0f;
            float duration = 0.0f;

            // Use defaulted copy/move operations to allow vector operations (push_back, erase, assignment).
            CactusGrowth() = default;
            CactusGrowth(const CactusGrowth&) = default;
            CactusGrowth& operator=(const CactusGrowth&) = default;
            CactusGrowth(CactusGrowth&&) noexcept = default;
            CactusGrowth& operator=(CactusGrowth&&) noexcept = default;
        };
        std::vector<CactusGrowth> cactusGrowths;

        // Helpers - return indexes instead of raw pointers to avoid exposing element addresses
        std::optional<std::size_t> findParticleSystemIndexByName(const std::string& name) const;
        std::vector<std::size_t> findObjectsIndexesByNameSubstring(const std::string& substr) const;

        void triggerGrowth();
    };
}