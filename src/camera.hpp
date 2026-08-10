#pragma once

#include <glm/glm.hpp>

namespace tg {

// Which directions the camera should move this frame.
struct CameraInput {
    bool forward = false;
    bool back    = false;
    bool left    = false;
    bool right   = false;
    bool up      = false;
    bool down    = false;
};

// FPS-style camera driven by delta-time movement and mouse look.
class Camera {
public:
    explicit Camera(glm::vec3 position, float yaw = -90.0f, float pitch = 0.0f);

    // Frame-rate independent movement. `dt` is in seconds.
    void update(const CameraInput& input, float dt) noexcept;

    // Mouse-look; offsets are in pixels.
    void rotate(float xOffset, float yOffset) noexcept;

    // Scroll wheel: glide along the current view direction.
    void glide(float amount) noexcept;

    // Preserve the same relative viewpoint when the terrain extent changes.
    void scalePosition(float factor) noexcept { position_ *= factor; }

    [[nodiscard]] glm::mat4 viewMatrix() const noexcept;
    [[nodiscard]] const glm::vec3& position() const noexcept { return position_; }

    bool  wireframe  = false;
    float moveSpeed  = 300.0f;  // world units per second
    float mouseSpeed = 0.05f;   // degrees per pixel

private:
    void updateFront() noexcept;

    glm::vec3 position_;
    glm::vec3 front_{0.0f, 0.0f, -1.0f};
    glm::vec3 up_{0.0f, 1.0f, 0.0f};
    float     yaw_   = -90.0f;
    float     pitch_ = 0.0f;
};

} // namespace tg
