#pragma once

// Thin move-only RAII wrappers over raw OpenGL handles, plus expected-based
// loaders for shader programs and BMP textures. No exceptions, no leaks,
// no manual glDelete* calls at call sites.

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <array>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

    ShaderProgram(ShaderProgram&& other) noexcept : id_(std::exchange(other.id_, 0)) {}
    ShaderProgram& operator=(ShaderProgram&& other) noexcept {
        if (this != &other) {
            reset();
            id_ = std::exchange(other.id_, 0);
        }
        return *this;
    }

    void use() const noexcept { glUseProgram(id_); }

    void set(std::string_view name, bool value)  const { set(name, static_cast<int>(value)); }
    void set(std::string_view name, int value)   const {
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
    }

    [[nodiscard]] GLint location(std::string_view name) const {
        return glGetUniformLocation(id_, std::string{name}.c_str());
    }

    GLuint id_ = 0;
};

namespace detail {

[[nodiscard]] inline std::expected<std::string, std::string>
readTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::unexpected("cannot open file: " + path.string());
    return std::string{std::istreambuf_iterator<char>{in},
                       std::istreambuf_iterator<char>{}};
}

[[nodiscard]] inline std::expected<GLuint, std::string>
compileShader(GLenum type, std::string_view source,
              const std::filesystem::path& path) {
    const GLuint shader = glCreateShader(type);
    const char*  ptr    = source.data();
    const GLint  length = static_cast<GLint>(source.size());
    glShaderSource(shader, 1, &ptr, &length);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<std::size_t>(logLength), '\0');
        glGetShaderInfoLog(shader, logLength, nullptr, log.data());
        glDeleteShader(shader);
        return std::unexpected("shader compilation failed (" + path.string() +
                               "):\n" + log);
    }
    return shader;
}

} // namespace detail

// Compile and link a shader program from two GLSL source files.
[[nodiscard]] inline std::expected<ShaderProgram, std::string>
loadProgram(const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath) {
    const auto vertexSource = detail::readTextFile(vertexPath);
    if (!vertexSource) return std::unexpected(vertexSource.error());
    const auto fragmentSource = detail::readTextFile(fragmentPath);
    if (!fragmentSource) return std::unexpected(fragmentSource.error());

    const auto vertexShader = detail::compileShader(GL_VERTEX_SHADER, *vertexSource, vertexPath);
    if (!vertexShader) return std::unexpected(vertexShader.error());
    const auto fragmentShader = detail::compileShader(GL_FRAGMENT_SHADER, *fragmentSource, fragmentPath);
    if (!fragmentShader) {
        glDeleteShader(*vertexShader);
        return std::unexpected(fragmentShader.error());
    }

    const GLuint program = glCreateProgram();
    glAttachShader(program, *vertexShader);
    glAttachShader(program, *fragmentShader);
    glLinkProgram(program);
    glDeleteShader(*vertexShader);
    glDeleteShader(*fragmentShader);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        GLint logLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        std::string log(static_cast<std::size_t>(logLength), '\0');
        glGetProgramInfoLog(program, logLength, nullptr, log.data());
        glDeleteProgram(program);
        return std::unexpected("shader program linking failed:\n" + log);
    }
    return ShaderProgram{program};
}

// Load a 24-bit uncompressed BMP as a mipmapped GL texture.
[[nodiscard]] inline std::expected<Texture2D, std::string>
loadBmpTexture(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::unexpected("cannot open texture: " + path.string());

    std::array<unsigned char, 54> header{};
    in.read(reinterpret_cast<char*>(header.data()),
            static_cast<std::streamsize>(header.size()));
    if (!in || header[0] != 'B' || header[1] != 'M')
        return std::unexpected("not a BMP file: " + path.string());

    const auto u32 = [&header](std::size_t offset) {
        return static_cast<std::uint32_t>(header[offset])        |
               static_cast<std::uint32_t>(header[offset + 1]) << 8u  |
               static_cast<std::uint32_t>(header[offset + 2]) << 16u |
               static_cast<std::uint32_t>(header[offset + 3]) << 24u;
    };
    std::uint32_t dataPos   = u32(0x0A);
    std::uint32_t imageSize = u32(0x22);
    const std::uint32_t width  = u32(0x12);
    const std::uint32_t height = u32(0x16);
    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0)   dataPos   = 54;

    std::vector<unsigned char> data(imageSize);
    in.seekg(static_cast<std::streamoff>(dataPos));
    in.read(reinterpret_cast<char*>(data.data()),
            static_cast<std::streamsize>(data.size()));
    if (!in)
        return std::unexpected("error reading image data: " + path.string());

    Texture2D texture;
    glBindTexture(GL_TEXTURE_2D, texture.id());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 static_cast<GLsizei>(width), static_cast<GLsizei>(height),
                 0, GL_BGR, GL_UNSIGNED_BYTE, data.data());
    glGenerateMipmap(GL_TEXTURE_2D);
    return texture;
}

} // namespace tg::gl
