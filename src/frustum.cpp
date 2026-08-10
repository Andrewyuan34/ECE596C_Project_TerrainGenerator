#include "frustum.hpp"

#include <cmath>

namespace tg {

Frustum Frustum::fromViewProjection(const glm::mat4& matrix) noexcept {
    const glm::vec4 row0{matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0]};
    const glm::vec4 row1{matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1]};
    const glm::vec4 row2{matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2]};
    const glm::vec4 row3{matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]};

    Frustum frustum;
    frustum.planes_ = {
        row3 + row0, row3 - row0,
        row3 + row1, row3 - row1,
        row3 + row2, row3 - row2,
    };
    for (glm::vec4& plane : frustum.planes_) {
        const float length = glm::length(glm::vec3{plane});
        if (length > 0.0f)
            plane /= length;
    }
    return frustum;
}

bool Frustum::intersectsAabb(const glm::vec3& boundsMin,
                             const glm::vec3& boundsMax) const noexcept {
    for (const glm::vec4& plane : planes_) {
        const glm::vec3 normal{plane};
        const glm::vec3 positive{
            normal.x >= 0.0f ? boundsMax.x : boundsMin.x,
            normal.y >= 0.0f ? boundsMax.y : boundsMin.y,
            normal.z >= 0.0f ? boundsMax.z : boundsMin.z,
        };
        if (glm::dot(normal, positive) + plane.w < 0.0f)
            return false;
    }
    return true;
}

} // namespace tg
