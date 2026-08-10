#pragma once

#include <array>
#include <expected>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

#include "camera.hpp"
#include "cli.hpp"
#include "gl_raii.hpp"
#include "terrain_mesh.hpp"

struct GLFWwindow;  // keep GLFW out of this header

namespace tg {

// Owns the window, all GPU resources and the main loop.
class Application {
public:
    explicit Application(CliOptions options);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    // Returns the process exit code.
    [[nodiscard]] int run();

private:
    struct EnvironmentSettings {
        float timeOfDay             = 17.5f;
        float maxSunElevationDegrees = 55.0f;
        float lightIntensity        = 1.0f;
    };

    struct EnvironmentState {
        glm::vec3 sunDirection{0.0f, 1.0f, 0.0f};
        glm::vec3 zenithColor{0.0f};
        glm::vec3 horizonColor{0.0f};
        glm::vec3 sunColor{0.0f};
        glm::vec3 ambientColor{0.0f};
        float     sunIntensity       = 0.0f;
        float     sunVisualIntensity = 0.0f;
    };

    struct WindowDeleter {
        void operator()(GLFWwindow* window) const;
    };

    // All objects whose destructors call glDelete* live here so they can be
    // destroyed together while the GL context is still current.
    struct GpuResources {
        gl::VertexArray   skyVao;
        gl::VertexArray   terrainVao;
        gl::Buffer        terrainVbo;
        gl::Buffer        terrainEbo;
        gl::ShaderProgram skyShader;
        gl::ShaderProgram terrainShader;
        gl::Texture2D     grassTexture;
        gl::Texture2D     sandTexture;
    };

    [[nodiscard]] std::expected<void, std::string> initWindow();
    [[nodiscard]] std::expected<void, std::string> loadAssets();
    [[nodiscard]] std::expected<void, std::string> initInterface();

    void mainLoop();
    void renderFrame();
    void renderControls();
    void uploadTerrainMesh();
    void updateTerrainUniforms();
    void regenerateTerrain();
    void updateEnvironment() noexcept;
    void updateWindowTitle(double now);
    [[nodiscard]] std::expected<void, std::string>
    saveScreenshot(const std::filesystem::path& path);

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void cursorPosCallback(GLFWwindow* window, double x, double y);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    CliOptions                              options_;
    std::unique_ptr<GLFWwindow, WindowDeleter> window_;
    std::optional<GpuResources>             gpu_;
    Camera                                  camera_;
    TerrainMesh                             mesh_;

    EnvironmentSettings environment_;
    EnvironmentState    environmentState_;

    double lastFrameTime_ = 0.0;
    double lastFpsTime_   = 0.0;
    int    frameCount_    = 0;
    double lastGenerationMs_ = 0.0;

    std::array<std::size_t, kTerrainLodCount> visibleLodCounts_{};
    std::size_t visibleChunkCount_    = 0;
    std::size_t visibleTriangleCount_ = 0;
    float lodNearDistance_ = 0.0f;
    float lodFarDistance_  = 0.0f;

    bool   imguiInitialized_   = false;
    bool   showControls_       = true;
    bool   frustumCulling_      = true;
    bool   distanceLod_         = true;
    bool   middleButtonPressed_ = false;
    bool   firstMouse_          = true;
    double lastMouseX_          = 0.0;
    double lastMouseY_          = 0.0;
};

} // namespace tg
