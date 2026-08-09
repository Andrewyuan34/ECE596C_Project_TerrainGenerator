# 重构决策记录

分支 `refactor/modern-cpp` 相对 `main`(原 ECE596C 版本)的关键决策与理由。

## 工程与环境

1. **便携工具链,项目内自包含**。MSVC 14.51 + Windows SDK 经
   [portable-msvc.py](https://gist.github.com/mmozeiko/7f3162ec2988e81e56d5c4e22cde9977)
   下载到 `.toolchain/msvc/`,配合便携 CMake / Ninja。全部位于项目目录、被
   `.gitignore` 排除,删除项目即完成卸载;重建步骤见 README。本机无需安装 Visual Studio。

2. **依赖管理用 FetchContent 而非 vcpkg/Conan**。GLFW 3.4、GLM 1.0.1、CLI11 2.4.2、
   glad2 均以固定版本 URL 声明在 `CMakeLists.txt` 中,下载与构建全部发生在 `build/` 内,
   零全局状态,clone 后一条 cmake 命令即可复现。

3. **不引入测试/CI/lint 设施**(按要求)。验证手段改为 `--screenshot` 离屏渲染冒烟测试。

## 语言与库

4. **C++23 为基线**(MSVC `/std:c++latest`)。实际用到的特性:
   `std::expected`(错误处理)、`std::mdspan`(二维高度图)、`std::print`/`std::format`、
   `std::span`(缓冲区上传)、指定初始化器、`constexpr` 数学函数、`[[nodiscard]]`。
   **刻意未用 C++ modules**:与 FetchContent 引入的第三方目标混编在当前工具链下仍不成熟,
   收益不抵复杂度(避免过度工程)。

5. **GLM 取代手写 Vec**。删除 `math.hpp`;`glm::lookAt` / `glm::perspective` 取代
   `gluLookAt` / `gluPerspective` 加 `glGetDoublev` 回读矩阵的固定管线写法。

6. **Boost.program_options → CLI11**。header-only、范围校验(`CLI::Range`)声明式内建,
   依赖体积与编译时间都显著下降。`parseCommandLine` 返回
   `std::expected<CliOptions, int>`,`--help` 与解析错误统一走 CLI11 的退出码约定。

## 架构

7. **CPU 网格生成与渲染完全解耦**。`terrain_mesh.{hpp,cpp}` 只产出
   `std::vector<Vertex>` + 索引,不含任何 GL 类型;GPU 上传全部在 `Application` 中。
   原代码中 Terrain 类既算噪声又持有 VAO/VBO 的混合职责被拆开。

8. **`Vertex` 聚合体取代平行数组**。原来的 `vertices`/`verticesWithNormals` 双数组手工
   交错改为 `struct Vertex { position; normal; uv; height; }`,并用
   `static_assert` 锁定内存布局与 `glVertexAttribPointer` 步长一致。

9. **OpenGL 句柄 RAII 化**。`gl_raii.hpp` 提供 move-only 的 `Buffer` / `VertexArray` /
   `Texture2D` / `ShaderProgram`,析构自动 `glDelete*`;所有 GPU 资源聚合进
   `std::optional<GpuResources>`,保证在 GL context 销毁之前统一释放(原代码靠
   `atexit` + 裸 `glDelete*`,顺序无保证)。`GL_CHECK` 宏整体删除。

10. **错误处理统一为 `std::expected`**,不再使用异常穿越模块边界、也不在库代码里
    `exit(1)`。着色器编译/链接失败、纹理读取失败、窗口/GL 初始化失败都返回带信息的
    `std::unexpected`,`main` 退出码因此可控。

## 渲染层(GLUT/GLEW/固定管线 → GLFW + glad2 + GL 3.3 Core)

11. **GLFW + glad2**:GLUT 回调模型换成 GLFW 回调 + 显式主循环;glewInit 换成
    `gladLoadGL`。着色器升级为 GLSL 330 core(`in`/`out`、`texture()`、
    `layout(location=N)` 显式 attribute 绑定,去掉运行时 `glGetAttribLocation`)。

12. **矩阵完全在 CPU 侧计算**,经 uniform 传入;删除 `glMatrixMode`/`glLoadIdentity`
    等固定管线 API。`gluBuild2DMipmaps` 换成 `glTexImage2D` + `glGenerateMipmap`。

13. **法线改由高度图中心差分计算**,取代"三角形法线累积再归一化":结果更平滑,
    且省掉一份 6 倍大小的临时法线数组。

## 交互与行为

14. **输入从"按一下动一格"改为轮询 + delta-time 连续移动**(WASD 按住平滑移动),
    光源 `2`/`3` 按住连续旋转;新增 `Esc` 退出、`Space`/`Shift` 升降别名;
    中键拖动视角保留。初始相机位置与俯角微调,默认参数下取景更完整。

15. **命令行参数语义与取值范围保持与原项目一致**(frequency/octave/amplitude/
    persistence/lacunarity/width/lod/seed),新增 `--window-width/--window-height` 与
    `--screenshot`(离屏渲染一帧存 BMP 后退出,用于无交互冒烟验证)。

16. **删除 `demo` bash 脚本**:它与旧 CLI 强绑定,其演示场景已由 README 的参数表与
    `--screenshot` 覆盖。

## 兼容性说明

- 地形生成公式(水位 35%、边缘抬升、高度缩放 worldSize/60、xz 缩放 0.1、纹理混合
  区间等)与原项目保持一致,同一组参数 + 同一种子生成视觉等价的地形。
- 仅支持 Windows + MSVC(按要求);OpenGL 3.3 Core 需要 2009 年后的任意显卡驱动。
