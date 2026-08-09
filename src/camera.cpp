#include "camera.hpp"

#include <algorithm>
#include <cmath>

#include <glm/gtc/matrix_transform.hpp>

namespace tg {

Camera::Camera(glm::vec3 position, float yaw, float pitch)
    : position_(position), yaw_(yaw), pitch_(pitch) {
    updateFront();
}

void Camera::update(const CameraInput& input, float dt) noexcept {
    const float velocity = moveSpeed * dt;

    // Horizontal forward: the view direction projected onto the XZ plane.
    const glm::vec3 frontH = glm::normalize(glm::vec3{front_.x, 0.0f, front_.z});
    const glm::vec3 right  = glm::normalize(glm::cross(frontH, up_));

    if (input.forward) position_ += frontH * velocity;
    if (input.back)    position_ -= frontH * velocity;
    if (input.left)    position_ -= right * velocity;
    if (input.right)   position_ += right * velocity;
    if (input.up)      position_ += up_ * velocity;
    if (input.down)    position_ -= up_ * velocity;
}

void Camera::rotate(float xOffset, float yOffset) noexcept {
    yaw_   += xOffset * mouseSpeed;
    pitch_ += yOffset * mouseSpeed;
    pitch_  = std::clamp(pitch_, -89.0f, 89.0f);  // avoid gimbal lock
    updateFront();
}

void Camera::glide(float amount) noexcept {
    position_ += front_ * amount;
}

glm::mat4 Camera::viewMatrix() const noexcept {
    return glm::lookAt(position_, position_ + front_, up_);
}

void Camera::updateFront() noexcept {
    front_ = glm::normalize(glm::vec3{
        std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_)),
        std::sin(glm::radians(pitch_)),
        std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_)),
    });
}

} // namespace tg
