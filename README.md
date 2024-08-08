# Terrain Generator

This program generates terrain using Perlin noise with customizable parameters.

## Command-line Options

- `-h, --help`: Produce help message.
- `-f, --frequency <arg>`: Set frequency. Range: 1~5, Step: 1. Default: 3.
- `-o, --octave <arg>`: Set octave. Range: 2~20, Step: 1. Default: 10.
- `-a, --amplitude <arg>`: Set amplitude. Range: 0.4~0.8, Step: 0.01. Default: 0.5.
- `-p, --persistence <arg>`: Set persistence. Range: 0.4~0.6, Step: 0.01. Default: 0.5.
- `-l, --lacunarity <arg>`: Set lacunarity. Range: 1~3, Step: 0.1. Default: 2.
- `-w, --width <arg>`: Set width. Range: 1~13, Step: 1. Default: 6.
- `-t, --step <arg>`: Set step. Range: 0~5, Step: 1. Default: 1.
- `-s, --seed <arg>`: Set seed. Default: 42.

## Controls

- **WSAD**: Horizontal movement (up, down, left, right).
- **R**: Move vertically up.
- **F**: Move vertically down.
- **1**: Toggle grid mode.
- **2**: Rotate light source clockwise.
- **3**: Rotate light source counterclockwise.
- **Mouse Scroll Wheel**: Move forward/backward along current view direction.
- **Hold Middle Mouse Button (Scroll Wheel)**: Control view direction by moving the mouse.

## Usage

1. Clone the repository: `git clone <repository_url>`
2. Navigate to the project directory: `cd cpp_project-yibo_yuan`
3. Build the project: `cmake -H. -B build`
4. Run the terrain generator with desired options: `./build/terrain_generator`

Then you can adjust the command-line options as needed to customize the generated terrain.


