#pragma once

#include <array>

#include <glm/glm.hpp>

namespace tg {

// Six normalized clipping planes extracted from a projection * view matrix.
class Frustum {
public:
    [[nodiscard]] static Frustum fromViewProjection(const glm::mat4& matrix) noexcept;

    [[nodiscard]] bool intersectsAabb(const glm::vec3& boundsMin,
                                      const glm::vec3& boundsMax) const noexcept;

private:
    std::array<glm::vec4, 6> planes_{};
};

} // namespace tg
