# Terrain Generator(Modern C++ 重构版)

使用 Perlin 噪声生成程序化地形,基于 **C++23 + OpenGL 3.3 Core** 完全重构。
原课程项目(ECE596C,GLUT/GLEW/Boost 版本)见 `main` 分支。

- 渲染:GLFW + glad2 + OpenGL 3.3 Core(自绘着色器管线,无固定管线)
- 数学:GLM
- 命令行:CLI11
- 构建:CMake(≥ 3.28)+ Ninja,依赖全部由 **FetchContent** 拉取
- 编译器:项目内**便携 MSVC**(无需安装 Visual Studio)

## 环境说明(重要)

本项目的整个工具链(MSVC、CMake、Ninja)和构建产物都位于项目目录内:

```
.toolchain/   # 便携 MSVC + CMake + Ninja(不入库,.gitignore 已排除)
build/        # CMake 构建目录,含 FetchContent 下载的第三方依赖
```

**没有任何全局安装**。删除项目文件夹即完成彻底卸载。

### 首次重建工具链

`.toolchain/` 不会被提交到远程。克隆后按以下步骤重建(约 500 MB 下载):

```bat
mkdir .toolchain
cd .toolchain

rem 1. 便携 MSVC + Windows SDK(mmozeiko/portable-msvc.py)
curl -L -o portable-msvc.py https://gist.githubusercontent.com/mmozeiko/7f3162ec2988e81e56d5c4e22cde9977/raw/portable-msvc.py
python portable-msvc.py --accept-license --target x64 --host x64

rem 2. 便携 CMake(解压后重命名为 cmake\)
curl -L -o cmake.zip https://github.com/Kitware/CMake/releases/download/v3.31.5/cmake-3.31.5-windows-x86_64.zip

rem 3. 便携 Ninja(解压到 ninja\)
curl -L -o ninja.zip https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip
```

glad2 还需要固定版本的 Python 依赖。解压上述工具后,将依赖安装到
项目内的本地工具链:

```bat
scripts\bootstrap-python.bat
```

> 网络受限时可先设置代理:`set https_proxy=http://127.0.0.1:7897`

## 构建与运行

```bat
scripts\env.bat
cmake --preset release
cmake --build --preset release
cd build
terrain_generator.exe
```

着色器和纹理会在构建后自动拷贝到可执行文件旁边,因此请在 `build\` 目录下运行。

## 命令行选项

| 选项 | 说明 | 范围 | 默认值 |
|---|---|---|---|
| `-h, --help` | 显示帮助 | | |
| `-f, --frequency` | 噪声频率 | 1 ~ 5 | 3 |
| `-o, --octave` | 倍频(octave)数 | 2 ~ 20 | 10 |
| `-a, --amplitude` | 振幅 | 0.4 ~ 0.8 | 0.5 |
| `-p, --persistence` | 持续度 | 0.4 ~ 0.6 | 0.5 |
| `-l, --lacunarity` | 间隙度 | 1 ~ 3 | 2 |
| `-w, --width` | 宽度倍率 | 1 ~ 13 | 6 |
| `-d, --lod` | 细节层级 | 0 ~ 5 | 1 |
| `-s, --seed` | 随机种子 | uint32 | 42 |
| `--window-width / --window-height` | 窗口尺寸 | | 1280 x 720 |
| `--screenshot <file.bmp>` | 离屏渲染一帧保存为 BMP 后退出(隐藏窗口,用于冒烟测试) | | |

## 操作方式

| 输入 | 行为 |
|---|---|
| `W` `S` `A` `D` | 水平移动(按住连续移动,帧率无关) |
| `R` / `Space` | 垂直上升 |
| `F` / `Left Shift` | 垂直下降 |
| 滚轮 | 沿视线方向前进/后退 |
| 按住鼠标中键拖动 | 视角旋转 |
| `1` | 切换线框模式 |
| `2` / `3`(按住) | 光源顺/逆时针旋转 |
| `Esc` | 退出 |

## 目录结构

```
src/
  main.cpp           入口:CLI -> Application
  cli.{hpp,cpp}      CLI11 命令行解析(std::expected 返回)
  perlin_noise.{hpp,cpp}  Perlin 噪声 + fBm
  terrain_mesh.{hpp,cpp}  纯 CPU 网格生成(不含任何 GL 类型)
  camera.{hpp,cpp}   FPS 相机(delta-time 移动 + 鼠标视角)
  gl_raii.hpp        OpenGL 句柄的 move-only RAII 封装 + expected 加载器
  application.{hpp,cpp}   窗口、输入、主循环、渲染
shader/              GLSL 330 core 着色器
texture/             BMP 纹理
scripts/env.bat      激活项目内工具链
```

重构决策记录见 [REFACTORING.md](REFACTORING.md)。
