#include "application.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <span>
#include <utility>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

namespace tg {

namespace {
constexpr glm::vec3 kAmbientLight{0.3f, 0.3f, 0.3f};
constexpr glm::vec3 kLightColor{1.0f, 1.0f, 1.0f};
constexpr float kLightRotationSpeed = 1.5f;   // radians per second while held
constexpr float kScrollGlideFactor  = 0.25f;  // fraction of moveSpeed per tick
constexpr int   kCubeIndexCount     = 36;

// Corners of a unit cube; scaled at render time through the model matrix.
constexpr float kCubeVertices[] = {
    -0.5f, -0.5f, -0.5f,  0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f, -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,  0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f, -0.5f,  0.5f,  0.5f,
};
constexpr std::uint32_t kCubeIndices[] = {
    0, 1, 2,  2, 3, 0,
    4, 5, 6,  6, 7, 4,
    0, 4, 7,  7, 3, 0,
    1, 5, 6,  6, 2, 1,
    3, 2, 6,  6, 7, 3,
    0, 1, 5,  5, 4, 0,
};
} // namespace

Application::Application(CliOptions options)
    : options_(std::move(options)),
      camera_(glm::vec3{0.0f, 300.0f, 700.0f}, -90.0f, -18.0f) {
    const float worldSize = static_cast<float>(1024 * options_.terrain.width);
    lightRadius_ = worldSize * 0.1f;
    lightHeight_ = worldSize / 30.0f;
    updateLight();
}

Application::~Application() {
    if (imguiInitialized_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
    gpu_.reset();     // GL objects go first, while the context is current
    window_.reset();  // then the window/context
    glfwTerminate();
}

void Application::WindowDeleter::operator()(GLFWwindow* window) const {
    if (window != nullptr) glfwDestroyWindow(window);
}

int Application::run() {
    if (const auto result = initWindow(); !result) {
        std::cerr << "error: " << result.error() << '\n';
        return 1;
    }
    if (const auto result = loadAssets(); !result) {
        std::cerr << "error: " << result.error() << '\n';
        return 1;
    }

    if (options_.screenshot) {
        renderFrame();
        if (const auto result = saveScreenshot(*options_.screenshot); !result) {
            std::cerr << "error: " << result.error() << '\n';
            return 1;
        }
        std::cout << "Screenshot saved to " << options_.screenshot->string() << '\n';
        return 0;
    }

    if (const auto result = initInterface(); !result) {
        std::cerr << "error: " << result.error() << '\n';
        return 1;
    }

    mainLoop();
    return 0;
}

std::expected<void, std::string> Application::initWindow() {
    glfwSetErrorCallback([](int code, const char* message) {
        std::cerr << "GLFW error " << code << ": " << message << '\n';
    });

    if (glfwInit() != GLFW_TRUE)
        return std::unexpected{"failed to initialize GLFW"};

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
    if (options_.screenshot)
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window_.reset(glfwCreateWindow(options_.windowWidth, options_.windowHeight,
                                   "Terrain Generator", nullptr, nullptr));
    if (!window_)
        return std::unexpected{"failed to create the GLFW window"};

    glfwMakeContextCurrent(window_.get());
    if (gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)) == 0)
        return std::unexpected{"failed to load OpenGL 3.3 Core functions"};

    glfwSwapInterval(1);
    glfwSetWindowUserPointer(window_.get(), this);
    glfwSetKeyCallback(window_.get(), keyCallback);
    glfwSetScrollCallback(window_.get(), scrollCallback);
    glfwSetCursorPosCallback(window_.get(), cursorPosCallback);
    glfwSetMouseButtonCallback(window_.get(), mouseButtonCallback);
    return {};
}

std::expected<void, std::string> Application::loadAssets() {
    mesh_ = generateTerrain(options_.terrain);
    gpu_.emplace();

    // --- Terrain mesh (terrain + water share one VAO / two index ranges) ---
    uploadTerrainMesh();

    // --- Light-source cube ---
    gpu_->cubeVao.bind();
    gpu_->cubeVbo.upload(GL_ARRAY_BUFFER, std::span<const float>{kCubeVertices});
    gpu_->cubeEbo.upload(GL_ELEMENT_ARRAY_BUFFER, std::span<const std::uint32_t>{kCubeIndices});
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    gl::VertexArray::unbind();

    // --- Shaders ---
    const auto shaderDir = options_.assetRoot / "shader";
    const auto textureDir = options_.assetRoot / "texture";
    auto terrainShader = gl::loadProgram(shaderDir / "terrain.vert",
                                         shaderDir / "terrain.frag");
    if (!terrainShader) return std::unexpected{std::move(terrainShader).error()};
    gpu_->terrainShader = std::move(*terrainShader);

    auto cubeShader = gl::loadProgram(shaderDir / "light_cube.vert",
                                      shaderDir / "light_cube.frag");
    if (!cubeShader) return std::unexpected{std::move(cubeShader).error()};
    gpu_->cubeShader = std::move(*cubeShader);

    // --- Textures ---
    auto grass = gl::loadBmpTexture(textureDir / "grass.bmp");
    if (!grass) return std::unexpected{std::move(grass).error()};
    gpu_->grassTexture = std::move(*grass);

    auto sand = gl::loadBmpTexture(textureDir / "sand.bmp");
    if (!sand) return std::unexpected{std::move(sand).error()};
    gpu_->sandTexture = std::move(*sand);

    // --- Static uniforms ---
    updateTerrainUniforms();
    gpu_->terrainShader.use();
    gpu_->terrainShader.set("uAmbientLight", kAmbientLight);
    gpu_->terrainShader.set("uGrassTexture", 0);
    gpu_->terrainShader.set("uSandTexture", 1);
    gpu_->cubeShader.use();
    gpu_->cubeShader.set("uLightColor", kLightColor);

    // --- Global GL state ---
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return {};
}

void Application::uploadTerrainMesh() {
    gpu_->terrainVao.bind();
    gpu_->terrainVbo.upload(GL_ARRAY_BUFFER, std::span{mesh_.vertices}, GL_DYNAMIC_DRAW);
    gpu_->terrainEbo.upload(GL_ELEMENT_ARRAY_BUFFER, std::span{mesh_.indices}, GL_DYNAMIC_DRAW);
    constexpr GLsizei stride = sizeof(Vertex);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, uv)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void*>(offsetof(Vertex, height)));
    gl::VertexArray::unbind();

    // GPU buffers now own the render data. Retain only mesh metadata on the
    // CPU so high LODs do not keep a second copy of large vertex/index arrays.
    std::vector<Vertex>{}.swap(mesh_.vertices);
    std::vector<std::uint32_t>{}.swap(mesh_.indices);
}

void Application::updateTerrainUniforms() {
    gpu_->terrainShader.use();
    gpu_->terrainShader.set("uWaterLevel", mesh_.waterLevel);
    gpu_->terrainShader.set("uHeightDifLow", mesh_.heightDifLow);
    gpu_->terrainShader.set("uHeightDifHigh", mesh_.heightDifHigh);
    gpu_->terrainShader.set("uWaterDepthMax", mesh_.waterDepthMax);
}

std::expected<void, std::string> Application::initInterface() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window_.get(), true)) {
        ImGui::DestroyContext();
        return std::unexpected{"failed to initialize the ImGui GLFW backend"};
    }
    if (!ImGui_ImplOpenGL3_Init("#version 330 core")) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        return std::unexpected{"failed to initialize the ImGui OpenGL backend"};
    }
    imguiInitialized_ = true;
    return {};
}

void Application::regenerateTerrain() {
    const double start = glfwGetTime();
    mesh_ = generateTerrain(options_.terrain);
    uploadTerrainMesh();
    updateTerrainUniforms();
    const float worldSize = static_cast<float>(mesh_.worldSize);
    lightRadius_ = worldSize * 0.1f;
    lightHeight_ = worldSize / 30.0f;
    updateLight();
    lastGenerationMs_ = (glfwGetTime() - start) * 1000.0;
}

void Application::mainLoop() {
    lastFrameTime_ = glfwGetTime();
    lastFpsTime_   = lastFrameTime_;

    while (!glfwWindowShouldClose(window_.get())) {
        glfwPollEvents();

        const double now = glfwGetTime();
        const float  dt  = static_cast<float>(now - lastFrameTime_);
        lastFrameTime_   = now;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        renderControls();

        // Continuous (held-key) input.
        GLFWwindow* w = window_.get();
        const auto pressed = [w](int key) { return glfwGetKey(w, key) == GLFW_PRESS; };
        if (!ImGui::GetIO().WantCaptureKeyboard) {
            camera_.update(CameraInput{
                .forward = pressed(GLFW_KEY_W),
                .back    = pressed(GLFW_KEY_S),
                .left    = pressed(GLFW_KEY_A),
                .right   = pressed(GLFW_KEY_D),
                .up      = pressed(GLFW_KEY_R) || pressed(GLFW_KEY_SPACE),
                .down    = pressed(GLFW_KEY_F) || pressed(GLFW_KEY_LEFT_SHIFT),
            }, dt);

            if (pressed(GLFW_KEY_2)) {
                lightAngle_ += kLightRotationSpeed * dt;
                updateLight();
            }
            if (pressed(GLFW_KEY_3)) {
                lightAngle_ -= kLightRotationSpeed * dt;
                updateLight();
            }
        }

        renderFrame();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(w);
        updateWindowTitle(now);
    }
}

void Application::renderControls() {
    if (!showControls_)
        return;

    ImGui::SetNextWindowSize(ImVec2{360.0f, 0.0f}, ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Terrain controls", &showControls_,
                      ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        return;
    }

    auto& terrain = options_.terrain;
    constexpr double frequencyMin = 1.0, frequencyMax = 5.0;
    constexpr double amplitudeMin = 0.4, amplitudeMax = 0.8;
    constexpr double persistenceMin = 0.4, persistenceMax = 0.6;
    constexpr double lacunarityMin = 1.0, lacunarityMax = 3.0;
    ImGui::SeparatorText("Noise");
    ImGui::SliderScalar("Frequency", ImGuiDataType_Double,
                        &terrain.noise.frequency, &frequencyMin, &frequencyMax, "%.2f");
    ImGui::SliderInt("Octaves", &terrain.noise.octaves, 2, 20);
    ImGui::SliderScalar("Amplitude", ImGuiDataType_Double,
                        &terrain.noise.amplitude, &amplitudeMin, &amplitudeMax, "%.2f");
    ImGui::SliderScalar("Persistence", ImGuiDataType_Double,
                        &terrain.noise.persistence, &persistenceMin, &persistenceMax, "%.2f");
    ImGui::SliderScalar("Lacunarity", ImGuiDataType_Double,
                        &terrain.noise.lacunarity, &lacunarityMin, &lacunarityMax, "%.2f");
    ImGui::InputScalar("Seed", ImGuiDataType_U32, &terrain.seed);

    ImGui::SeparatorText("Mesh");
    ImGui::SliderInt("Width", &terrain.width, 1, 13);
    ImGui::SliderInt("Detail", &terrain.lod, 0, 5);

    ImGui::SeparatorText("View");
    ImGui::Checkbox("Wireframe", &camera_.wireframe);
    ImGui::SliderFloat("Camera speed", &camera_.moveSpeed, 25.0f, 1500.0f, "%.0f");

    if (ImGui::Button("Regenerate terrain"))
        regenerateTerrain();
    ImGui::SameLine();
    ImGui::TextDisabled("F1 hides this panel");
    ImGui::Text("Triangles: %zu", mesh_.terrainIndexCount / 3u);
    if (lastGenerationMs_ > 0.0)
        ImGui::Text("Last generation: %.1f ms", lastGenerationMs_);
    ImGui::End();
}

void Application::renderFrame() {
    int fbWidth = 0, fbHeight = 0;
    glfwGetFramebufferSize(window_.get(), &fbWidth, &fbHeight);
    if (fbWidth == 0 || fbHeight == 0)
        return;  // minimized

    glViewport(0, 0, fbWidth, fbHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(fbWidth) / static_cast<float>(fbHeight);
    const glm::mat4 viewProj =
        glm::perspective(glm::radians(45.0f), aspect, 10.0f, 10000.0f) *
        camera_.viewMatrix();

    // --- Terrain, then water (second index range, alpha-blended) ---
    auto& shader = gpu_->terrainShader;
    shader.use();
    shader.set("uViewProj", viewProj);
    shader.set("uLightPos", lightPos_);
    glPolygonMode(GL_FRONT_AND_BACK, camera_.wireframe ? GL_LINE : GL_FILL);
    gpu_->grassTexture.bind(GL_TEXTURE0);
    gpu_->sandTexture.bind(GL_TEXTURE1);
    gpu_->terrainVao.bind();

    shader.set("uDrawWater", false);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh_.terrainIndexCount),
                   GL_UNSIGNED_INT, nullptr);

    shader.set("uDrawWater", true);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(mesh_.terrainIndexCount),
                   GL_UNSIGNED_INT, nullptr);
    gl::VertexArray::unbind();

    // --- Light-source cube (always visible: no culling, no depth test) ---
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    gpu_->cubeShader.use();
    const glm::mat4 model =
        glm::translate(glm::mat4{1.0f}, lightPos_) *
        glm::scale(glm::mat4{1.0f}, glm::vec3{static_cast<float>(options_.terrain.width)});
    gpu_->cubeShader.set("uModel", model);
    gpu_->cubeShader.set("uViewProj", viewProj);
    gpu_->cubeVao.bind();
    glDrawElements(GL_TRIANGLES, kCubeIndexCount, GL_UNSIGNED_INT, nullptr);
    gl::VertexArray::unbind();
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void Application::updateLight() noexcept {
    lightPos_ = {lightRadius_ * std::cos(lightAngle_),
                 lightHeight_,
                 lightRadius_ * std::sin(lightAngle_)};
}

void Application::updateWindowTitle(double now) {
    ++frameCount_;
    if (now - lastFpsTime_ >= 1.0) {
        const double fps = frameCount_ / (now - lastFpsTime_);
        frameCount_  = 0;
        lastFpsTime_ = now;
        std::ostringstream title;
        title << "Terrain Generator - FPS: " << std::fixed << std::setprecision(0) << fps;
        glfwSetWindowTitle(window_.get(), title.str().c_str());
    }
}

std::expected<void, std::string>
Application::saveScreenshot(const std::filesystem::path& path) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window_.get(), &width, &height);
    if (width <= 0 || height <= 0)
        return std::unexpected{"cannot capture an empty framebuffer"};

    const int rowSize   = width * 3;
    const int paddedRow = (rowSize + 3) & ~3;  // BMP rows are 4-byte aligned

    std::vector<unsigned char> raw(static_cast<std::size_t>(rowSize) * height);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_BGR, GL_UNSIGNED_BYTE, raw.data());
    if (const GLenum error = glGetError(); error != GL_NO_ERROR)
        return std::unexpected{"OpenGL readback failed with error " +
                               std::to_string(error)};

    // glReadPixels and BMP both store rows bottom-up: copy straight through.
    std::vector<unsigned char> pixels(static_cast<std::size_t>(paddedRow) * height, 0);
    for (int r = 0; r < height; ++r)
        std::copy_n(raw.data() + static_cast<std::size_t>(r) * rowSize, rowSize,
                    pixels.data() + static_cast<std::size_t>(r) * paddedRow);

    std::ofstream out(path, std::ios::binary);
    if (!out)
        return std::unexpected{"cannot write screenshot to " + path.string()};

    std::array<unsigned char, 54> header{};
    const auto put32 = [&header](std::size_t off, std::uint32_t v) {
        header[off]     = static_cast<unsigned char>(v);
        header[off + 1] = static_cast<unsigned char>(v >> 8u);
        header[off + 2] = static_cast<unsigned char>(v >> 16u);
        header[off + 3] = static_cast<unsigned char>(v >> 24u);
    };
    const auto put16 = [&header](std::size_t off, std::uint16_t v) {
        header[off]     = static_cast<unsigned char>(v);
        header[off + 1] = static_cast<unsigned char>(v >> 8u);
    };
    header[0] = 'B';
    header[1] = 'M';
    put32(0x02, 54u + static_cast<std::uint32_t>(pixels.size()));
    put32(0x0A, 54u);
    put32(0x0E, 40u);
    put32(0x12, static_cast<std::uint32_t>(width));
    put32(0x16, static_cast<std::uint32_t>(height));
    put16(0x1A, 1);
    put16(0x1C, 24);
    put32(0x22, static_cast<std::uint32_t>(pixels.size()));

    out.write(reinterpret_cast<const char*>(header.data()),
              static_cast<std::streamsize>(header.size()));
    out.write(reinterpret_cast<const char*>(pixels.data()),
              static_cast<std::streamsize>(pixels.size()));
    if (!out)
        return std::unexpected{"failed while writing screenshot to " + path.string()};
    return {};
}

// --- GLFW callback trampolines ----------------------------------------------

void Application::keyCallback(GLFWwindow* window, int key, int, int action, int) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (action != GLFW_PRESS)
        return;
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
        return;
    }
    if (key == GLFW_KEY_F1) {
        app->showControls_ = !app->showControls_;
        return;
    }
    if (app->imguiInitialized_ && ImGui::GetIO().WantCaptureKeyboard)
        return;
    switch (key) {
        case GLFW_KEY_1:
            app->camera_.wireframe = !app->camera_.wireframe;
            break;
        default:
            break;
    }
}

void Application::scrollCallback(GLFWwindow* window, double, double yoffset) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app->imguiInitialized_ && ImGui::GetIO().WantCaptureMouse)
        return;
    app->camera_.glide(static_cast<float>(yoffset) * app->camera_.moveSpeed *
                       kScrollGlideFactor);
}

void Application::cursorPosCallback(GLFWwindow* window, double x, double y) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (app->imguiInitialized_ && ImGui::GetIO().WantCaptureMouse)
        return;
    if (!app->middleButtonPressed_)
        return;
    if (app->firstMouse_) {
        app->lastMouseX_ = x;
        app->lastMouseY_ = y;
        app->firstMouse_ = false;
    }
    const float xOffset = static_cast<float>(x - app->lastMouseX_);
    const float yOffset = static_cast<float>(app->lastMouseY_ - y);
    app->lastMouseX_ = x;
    app->lastMouseY_ = y;
    app->camera_.rotate(xOffset, yOffset);
}

void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int) {
    auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    if (button != GLFW_MOUSE_BUTTON_MIDDLE)
        return;
    if (action == GLFW_RELEASE) {
        app->middleButtonPressed_ = false;
        return;
    }
    if (app->imguiInitialized_ && ImGui::GetIO().WantCaptureMouse)
        return;
    app->middleButtonPressed_ = (action == GLFW_PRESS);
    if (app->middleButtonPressed_) {
        app->firstMouse_ = true;
        glfwGetCursorPos(window, &app->lastMouseX_, &app->lastMouseY_);
    }
}

} // namespace tg
