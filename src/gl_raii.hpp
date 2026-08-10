#pragma once

// Thin move-only RAII wrappers over raw OpenGL handles, plus expected-based
// loaders for shader programs and BMP textures. No exceptions, no leaks,
// no manual glDelete* calls at call sites.

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <expected>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>

namespace tg::gl {

class Buffer {
public:
    Buffer() noexcept { glGenBuffers(1, &id_); }
    ~Buffer() { if (id_ != 0) glDeleteBuffers(1, &id_); }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteBuffers(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    [[nodiscard]] GLuint id() const noexcept { return id_; }

    template <typename T>
    void upload(GLenum target, std::span<T> data,
                GLenum usage = GL_STATIC_DRAW) const noexcept {
        glBindBuffer(target, id_);
        glBufferData(target, static_cast<GLsizeiptr>(data.size_bytes()),
                     data.data(), usage);
    }

private:
    GLuint id_ = 0;
};

class VertexArray {
public:
    VertexArray() noexcept { glGenVertexArrays(1, &id_); }
    ~VertexArray() { if (id_ != 0) glDeleteVertexArrays(1, &id_); }

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    VertexArray(VertexArray&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
    VertexArray& operator=(VertexArray&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteVertexArrays(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    [[nodiscard]] GLuint id() const noexcept { return id_; }

    void bind() const noexcept { glBindVertexArray(id_); }
    static void unbind() noexcept { glBindVertexArray(0); }

private:
    GLuint id_ = 0;
};

class Texture2D {
public:
    Texture2D() noexcept { glGenTextures(1, &id_); }
    ~Texture2D() { if (id_ != 0) glDeleteTextures(1, &id_); }

    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;

    Texture2D(Texture2D&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
    Texture2D& operator=(Texture2D&& other) noexcept {
        if (this != &other) {
            if (id_ != 0) glDeleteTextures(1, &id_);
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    [[nodiscard]] GLuint id() const noexcept { return id_; }

    void bind(GLenum unit = GL_TEXTURE0) const noexcept {
        glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_2D, id_);
    }

private:
    GLuint id_ = 0;
};

class ShaderProgram {
public:
    ShaderProgram() = default;
    ~ShaderProgram() { reset(); }

    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&& other) noexcept
        : id_(std::exchange(other.id_, 0)),
          uniformLocations_(std::move(other.uniformLocations_)) {}
    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            reset();
            id_ = std::exchange(other.id_, 0);
            uniformLocations_ = std::move(other.uniformLocations_);
        }
        return *this;
    }

    void use() const noexcept { glUseProgram(id_); }

    void set(std::string_view name, bool value) const {
        set(name, static_cast<int>(value));
    }
    void set(std::string_view name, int value) const {
        glUniform1i(location(name), value);
    }
    void set(std::string_view name, float value) const {
        glUniform1f(location(name), value);
    }
    void set(std::string_view name, const glm::vec3& value) const {
        glUniform3fv(location(name), 1, glm::value_ptr(value));
    }
    void set(std::string_view name, const glm::mat4& value) const {
        glUniformMatrix4fv(location(name), 1, GL_FALSE, glm::value_ptr(value));
    }

private:
    friend std::expected<ShaderProgram, std::string>
    loadProgram(const std::filesystem::path& vertexPath,
                const std::filesystem::path& fragmentPath);

    explicit ShaderProgram(GLuint id) noexcept : id_(id) {}

    void reset() noexcept {
        if (id_ != 0) glDeleteProgram(id_);
        id_ = 0;
        uniformLocations_.clear();
    }

    [[nodiscard]] GLint location(std::string_view name) const {
        auto [entry, inserted] = uniformLocations_.try_emplace(std::string{name}, -1);
        if (inserted)
            entry->second = glGetUniformLocation(id_, entry->first.c_str());
        return entry->second;
    }

    GLuint id_ = 0;
    mutable std::unordered_map<std::string, GLint> uniformLocations_;
};

[[nodiscard]] std::expected<ShaderProgram, std::string>
loadProgram(const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath);

[[nodiscard]] std::expected<Texture2D, std::string>
loadBmpTexture(const std::filesystem::path& path);

} // namespace tg::gl
