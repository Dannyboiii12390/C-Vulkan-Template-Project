#include "ParticleManager.h"
#include <algorithm>
#include <random>

namespace Engine
{
    void ParticleManager::init(std::vector<Engine::ParticleSystem>* particleSystems_,
        std::vector<Engine::Object>* objects_)
    {
        particleSystems = particleSystems_;
        objects = objects_;
    }

    std::optional<std::size_t> ParticleManager::findParticleSystemIndexByName(const std::string& name) const
    {
        if (!particleSystems) return std::nullopt;
        for (std::size_t i = 0; i < particleSystems->size(); ++i)
        {
            if (std::string((*particleSystems)[i].getName()) == name) return i;
        }
        return std::nullopt;
    }

    std::vector<std::size_t> ParticleManager::findObjectsIndexesByNameSubstring(const std::string& substr) const
    {
        std::vector<std::size_t> result;
        if (!objects) return result;
        for (std::size_t i = 0; i < objects->size(); ++i)
        {
            const std::string name = std::string((*objects)[i].getName());
            if (name.find(substr) != std::string::npos) result.push_back(i);
        }
        return result;
    }
    void ParticleManager::triggerGrowth()
    {
        auto cactiIdx = findObjectsIndexesByNameSubstring("cactus");
        for (const auto idx : cactiIdx)
        {
            if (!objects || idx >= objects->size()) continue;
            CactusGrowth growth;
            growth.objIndex = idx;
            growth.startScale = (*objects)[idx].getScale();
            growth.targetScale = growth.startScale * cactusGrowthMultiplier; // grow by multiplier
            growth.elapsed = 0.0f;
            growth.duration = defaultGrowthDuration;
            cactusGrowths.push_back(growth);
        }
        precipEndedWaitingForSun = false;

    }

    void ParticleManager::update(float deltaTime, const Engine::UniformBufferObject& ubo)
    {
		snowProbability += chanceIncrement * deltaTime;
		snowProbability = glm::clamp(snowProbability, 0.0f, 1.0f);

        if (!particleSystems || !objects) return;

        // Determine rain/snow state by checking the corresponding particle systems
        std::optional<std::size_t> rainIdx = findParticleSystemIndexByName("RainParticleSystem");
        Engine::ParticleSystem* rainPS = nullptr;
        bool currentlyRaining = false;
        if (rainIdx && rainIdx.value() < particleSystems->size())
        {
            rainPS = &((*particleSystems)[rainIdx.value()]);
            currentlyRaining = rainPS->isActive();
        }

        std::optional<std::size_t> snowIdx = findParticleSystemIndexByName("SnowParticleSystem");
        Engine::ParticleSystem* snowPS = nullptr;
        bool currentlySnowing = false;
        if (snowIdx && snowIdx.value() < particleSystems->size())
        {
            snowPS = &((*particleSystems)[snowIdx.value()]);
            currentlySnowing = snowPS->isActive();
        }

        // If rain just started, occasionally replace it by snow (tunable probability)
        if (currentlyRaining && !rainWasActive)
        {
            // Use a thread-local RNG to avoid reseeding each frame
            static thread_local std::mt19937 rng{ std::random_device{}() };
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            const float r = dist(rng);
            if (r < snowProbability && snowPS && rainPS)
            {
                // turn rain off and enable snow instead
                rainPS->setActive(false);
                snowPS->setActive(true);
                currentlyRaining = false;
                currentlySnowing = true;
				snowProbability = 0.0f; // reset chance accumulator

                // Important: record that precipitation was active so that when it ends
                // we still detect the end and trigger cactus growth. Converting rain->snow
                // mid-frame could otherwise leave previous-state flags unset and prevent
                // the "precipitation ended" detection later.
                rainWasActive = true;
                snowWasActive = true;
            }
        }

        // Any precipitation (rain OR snow)
        const bool currentlyPrecipitating = currentlyRaining || currentlySnowing;

        // If snow is active, forcibly ensure fire system is off and reset sunshine accumulator.
        if (currentlySnowing)
        {
            std::optional<std::size_t> fireIdx = findParticleSystemIndexByName("FireParticleSystem");
            if (fireIdx && fireIdx.value() < particleSystems->size())
            {
                Engine::ParticleSystem* const firePS = &((*particleSystems)[fireIdx.value()]);
                if (firePS && firePS->isActive())
                {
                    firePS->setActive(false);
                }
            }
            fireActive = false;
            sunshineAccumulator = 0.0f;
        }

        // Determine whether the sun is "above the horizon" and shining.
        // Snowing blocks sunlight (no burning while snowing).
        const float sunIntensityThreshold = 0.01f;
        const bool sunIsShining = (ubo.sun_pos.y > 0.0f) && (ubo.sun_intensity > sunIntensityThreshold) && !currentlySnowing;

        // Accumulate sunshine only if sun is shining and there is no precipitation
        if (sunIsShining && !currentlyPrecipitating)
        {
            sunshineAccumulator += deltaTime;
        }
        else
        {
            sunshineAccumulator = 0.0f;
        }

        // Start fire if we've had enough continuous sunshine and fire is not already active
        if (sunshineAccumulator >= sunshineThreshold && !fireActive && !currentlyPrecipitating)
        {
            std::optional<std::size_t> fireIdx = findParticleSystemIndexByName("FireParticleSystem");
            if (fireIdx && fireIdx.value() < particleSystems->size())
            {
                Engine::ParticleSystem* const firePS = &((*particleSystems)[fireIdx.value()]);
                if (firePS)
                {
                    firePS->setActive(true);
                    fireActive = true;
                }
            }
        }

        // If precipitation starts (rain OR snow), extinguish any active fire
        if (currentlyPrecipitating && fireActive)
        {
            std::optional<std::size_t> fireIdx = findParticleSystemIndexByName("FireParticleSystem");
            if (fireIdx && fireIdx.value() < particleSystems->size())
            {
                Engine::ParticleSystem* const firePS = &((*particleSystems)[fireIdx.value()]);
                if (firePS)
                {
                    firePS->setActive(false);
                    fireActive = false;
                }
            }
        }

        // Detect precipitation end -> trigger cactus growth (only when sun is shining).
        // If precipitation ends but sun is not shining yet, defer growth until sun appears.
        const bool precipWasActive = rainWasActive || snowWasActive;
        if (precipWasActive && !currentlyPrecipitating)
        {
            if (sunIsShining)
            {
                // immediate growth trigger
                triggerGrowth();
            }
            else
            {
                // wait until sun appears
                precipEndedWaitingForSun = true;
            }
        }

        // If precipitation previously ended and we were waiting for sun, start growth when sun appears
        if (precipEndedWaitingForSun && sunIsShining && !currentlyPrecipitating)
        {
            triggerGrowth();
        }

        // Update growth animations
        for (size_t i = 0; i < cactusGrowths.size(); /*incremented in loop*/)
        {
            auto& g = cactusGrowths[i];
            g.elapsed += deltaTime;
            const float t = glm::clamp(g.elapsed / g.duration, 0.0f, 1.0f);
            const glm::vec3 newScale = glm::mix(g.startScale, g.targetScale, t);
            if (objects && g.objIndex != static_cast<std::size_t>(-1) && g.objIndex < objects->size())
            {
                (*objects)[g.objIndex].setScale(newScale);
            }

            if (t >= 1.0f)
            {
                // finished, remove entry
                cactusGrowths.erase(cactusGrowths.begin() + i);
            }
            else
            {
                ++i;
            }
        }

        // store precipitation state
        rainWasActive = currentlyRaining;
        snowWasActive = currentlySnowing;
    }
}