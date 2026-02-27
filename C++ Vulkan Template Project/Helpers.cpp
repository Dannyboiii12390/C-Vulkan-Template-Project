#include "Helpers.h"

void Helpers::computeShadowProjection(
    const glm::vec3& sunDir,
    const glm::vec3& basePos,
    float objectHeight,
    float objectRadius,
    glm::vec2& outScaleXZ,
    glm::vec2& outCenterOffsetXZ,
    float meshPivot) // 0 = origin at near edge, 0.5 = origin centered
{
    // Initialize outputs
    outScaleXZ = glm::vec2(1.0f);
    outCenterOffsetXZ = glm::vec2(0.0f);

    const float EPS = 1e-6f;
    if (glm::length(sunDir) < EPS) return;

    const glm::vec3 ray = glm::normalize(sunDir);

    // elevation in [0..1] (0=horizon, 1=overhead)
    const float elevation = glm::clamp(std::abs(ray.y), 0.0f, 1.0f);

    // Smoothly blend maximum shadow length based on elevation (normalized thresholds)
    const float fadeStart = 0.02f;
    const float fadeFull = 0.5f;
    const float fade = (fadeFull > fadeStart) ? glm::clamp((elevation - fadeStart) / (fadeFull - fadeStart), 0.0f, 1.0f) : 1.0f;

    const float maxNearHorizon = glm::max(5.0f, objectRadius * 1.0f);
    const float maxHighSun = glm::max(50.0f, objectRadius * 20.0f);
    const float maxL = glm::mix(maxNearHorizon, maxHighSun, fade);

    // If the ray is nearly parallel to the ground, choose a stable fallback direction and clamp length
    if (std::abs(ray.y) < EPS)
    {
        const glm::vec2 dirXZ = glm::normalize(glm::vec2(ray.x, ray.z));
        const float L = maxL;
        // Position the mesh origin by fraction meshPivot along the projected vector
        outCenterOffsetXZ = dirXZ * (meshPivot * L);

        const float baseRef = glm::max(objectRadius, 0.001f);
        const float scaleFactor = 1.0f + (L / (baseRef * 5.0f));
        outScaleXZ = glm::vec2(glm::clamp(scaleFactor, 0.01f, 10.0f));
        return;
    }

    // Compute where the top of the object projects down to the ground plane at basePos.y
    const glm::vec3 topPos = basePos + glm::vec3(0.0f, objectHeight, 0.0f);
    const float t = (basePos.y - topPos.y) / ray.y;

    // If t <= 0 then the projection in the ray direction doesn't hit the ground in front of the object
    if (t <= 0.0f)
    {
        // Keep default outputs (small, local shadow)
        return;
    }

    const glm::vec3 projTop = topPos + t * ray;
    const glm::vec2 deltaXZ = glm::vec2(projTop.x - basePos.x, projTop.z - basePos.z);
    const float Lraw = glm::length(deltaXZ);

    // clamp length to avoid runaway values near the horizon
    const float L = glm::clamp(Lraw, 0.0f, maxL);

    const glm::vec2 dirXZ = (Lraw > EPS) ? (deltaXZ / Lraw) : glm::vec2(0.0f, -1.0f);

    // Position the shadow quad origin: move from the object's base toward projTop
    // by the meshPivot fraction of the projected length.
    outCenterOffsetXZ = dirXZ * (meshPivot * L);

    // Safety clamp to maxL
    const float len = glm::length(outCenterOffsetXZ);
    if (len > maxL && len > EPS)
    {
        outCenterOffsetXZ = (outCenterOffsetXZ / len) * maxL;
    }

    // Scale shadow based on projected length relative to object radius
    const float baseRef = glm::max(objectRadius, 0.001f);
    float scaleFactor = 1.0f + (L / (baseRef * 5.0f));
    scaleFactor = glm::clamp(scaleFactor, 0.01f, 10.0f);
    outScaleXZ = glm::vec2(scaleFactor, scaleFactor);
}