#pragma once
#include <glm/glm.hpp>


class Helpers final
{
public:
    static void computeShadowProjection(
        const glm::vec3& sunDir,
        const glm::vec3& basePos,
        float objectHeight,
        float objectRadius,
        glm::vec2& outScaleXZ,
        glm::vec2& outCenterOffsetXZ,
        float meshPivot = 0.5f);
};

