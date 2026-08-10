#include "gl_raii.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <limits>
#include <vector>

namespace tg::gl {
namespace {

[[nodiscard]] std::expected<std::string, std::string>
readTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::unexpected("cannot open file: " + path.string());
    return std::string{std::istreambuf_iterator<char>{in},
                       std::istreambuf_iterator<char>{}};
}

[[nodiscard]] std::expected<GLuint, std::string>
compileShader(GLenum type, std::string_view source,
              const std::filesystem::path& path) {
    const GLuint shader = glCreateShader(type);
    const char* ptr = source.data();
    const GLint length = static_cast<GLint>(source.size());
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

} // namespace

std::expected<ShaderProgram, std::string>
loadProgram(const std::filesystem::path& vertexPath,
            const std::filesystem::path& fragmentPath) {
    const auto vertexSource = readTextFile(vertexPath);
    if (!vertexSource)
        return std::unexpected(vertexSource.error());
    const auto fragmentSource = readTextFile(fragmentPath);
    if (!fragmentSource)
        return std::unexpected(fragmentSource.error());

    const auto vertexShader = compileShader(GL_VERTEX_SHADER, *vertexSource, vertexPath);
    if (!vertexShader)
        return std::unexpected(vertexShader.error());
    const auto fragmentShader = compileShader(GL_FRAGMENT_SHADER, *fragmentSource,
                                              fragmentPath);
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

std::expected<Texture2D, std::string>
loadBmpTexture(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in)
        return std::unexpected("cannot open texture: " + path.string());

    std::array<unsigned char, 54> header{};
    in.read(reinterpret_cast<char*>(header.data()),
            static_cast<std::streamsize>(header.size()));
    if (!in || header[0] != 'B' || header[1] != 'M')
        return std::unexpected("not a BMP file: " + path.string());

    const auto u16 = [&header](std::size_t offset) {
        return static_cast<std::uint16_t>(header[offset]) |
               static_cast<std::uint16_t>(header[offset + 1]) << 8u;
    };
    const auto u32 = [&header](std::size_t offset) {
        return static_cast<std::uint32_t>(header[offset]) |
               static_cast<std::uint32_t>(header[offset + 1]) << 8u |
               static_cast<std::uint32_t>(header[offset + 2]) << 16u |
               static_cast<std::uint32_t>(header[offset + 3]) << 24u;
    };

    const std::uint32_t dataPos = u32(0x0A);
    const std::uint32_t dibSize = u32(0x0E);
    const auto widthSigned = static_cast<std::int32_t>(u32(0x12));
    const auto heightSigned = static_cast<std::int32_t>(u32(0x16));
    const std::uint32_t declaredImageSize = u32(0x22);
    if (dibSize < 40 || dataPos < 14u + dibSize)
        return std::unexpected("unsupported BMP header: " + path.string());
    if (u16(0x1A) != 1 || u16(0x1C) != 24 || u32(0x1E) != 0)
        return std::unexpected("texture must be an uncompressed 24-bit BMP: " +
                               path.string());
    if (widthSigned <= 0 || heightSigned == 0)
        return std::unexpected("invalid BMP dimensions: " + path.string());

    constexpr std::uint32_t kMaxTextureDimension = 16'384;
    const std::uint64_t width = static_cast<std::uint32_t>(widthSigned);
    const std::int64_t signedHeight = heightSigned;
    const std::uint64_t height = static_cast<std::uint64_t>(
        signedHeight < 0 ? -signedHeight : signedHeight);
    if (width > kMaxTextureDimension || height > kMaxTextureDimension)
        return std::unexpected("BMP dimensions exceed the supported limit: " +
                               path.string());

    const std::uint64_t packedRow = width * 3u;
    const std::uint64_t fileRow = (packedRow + 3u) & ~std::uint64_t{3u};
    const std::uint64_t storedSize = fileRow * height;
    if (declaredImageSize != 0 && declaredImageSize < storedSize)
        return std::unexpected("BMP image size is truncated: " + path.string());

    std::error_code sizeError;
    const std::uintmax_t fileSize = std::filesystem::file_size(path, sizeError);
    if (sizeError || dataPos > fileSize || storedSize > fileSize - dataPos)
        return std::unexpected("BMP pixel data exceeds the file: " + path.string());
    if (storedSize > std::numeric_limits<std::size_t>::max())
        return std::unexpected("BMP is too large to load: " + path.string());

    std::vector<unsigned char> stored(static_cast<std::size_t>(storedSize));
    in.seekg(static_cast<std::streamoff>(dataPos));
    in.read(reinterpret_cast<char*>(stored.data()),
            static_cast<std::streamsize>(stored.size()));
    if (!in)
        return std::unexpected("error reading image data: " + path.string());

    std::vector<unsigned char> data(static_cast<std::size_t>(packedRow * height));
    const bool topDown = heightSigned < 0;
    for (std::size_t row = 0; row < static_cast<std::size_t>(height); ++row) {
        const std::size_t sourceRow = topDown
            ? static_cast<std::size_t>(height) - 1u - row
            : row;
        std::copy_n(stored.data() + sourceRow * static_cast<std::size_t>(fileRow),
                    static_cast<std::size_t>(packedRow),
                    data.data() + row * static_cast<std::size_t>(packedRow));
    }

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
