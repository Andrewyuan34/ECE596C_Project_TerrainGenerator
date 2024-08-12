Terrain Generator

This program generates terrain using Perlin noise with customizable parameters.

Command-line Options:

- -h, --help: Produce help message.
- -f, --frequency <arg>: Set frequency. Range: 1~5, Step: 1. Default: 3. tips: around 3 looks good
- -o, --octave <arg>: Set octave. Range: 2~20, Step: 1. Default: 10. tips: around 10 looks good
- -a, --amplitude <arg>: Set amplitude. Range: 0.4~0.8, Step: 0.01. Default: 0.5. tips: around 0.5 looks good
- -p, --persistence <arg>: Set persistence. Range: 0.4~0.6, Step: 0.01. Default: 0.5. tips: around 0.5 looks good
- -l, --lacunarity <arg>: Set lacunarity. Range: 1~3, Step: 0.1. Default: 2. tips: around 2 looks good
- -w, --width <arg>: Set width. Range: 1~13, Step: 1. Default: 6. tips: The larger the width, the bigger the terrain
- -t, --level of detail <arg>: Set level of detail. Range: 0~5, Step: 1. Default: 1. tips: The larger the LOD, the more detailed the terrain
- -s, --seed <arg>: Set seed. Default: 42.

Controls:

- 'W''S''A''D': Horizontal movement (forward, backward, left, right).
- 'R': Move vertically up.
- 'F': Move vertically down.
- '1': Toggle grid mode.
- '2': Rotate light source clockwise.
- '3': Rotate light source counterclockwise.
- Mouse Scroll Wheel: Move forward/backward along current view direction.
- Hold Middle Mouse Button (Scroll Wheel): Control view direction by moving the mouse.

Usage:

1. Clone the repository: git clone <repository_url>
2. Navigate to the project directory: cd cpp_project-yibo_yuan
3. Configure the project: cmake -H. -B build
4. Build the project: cmake --build build
5. Run the terrain generator with desired options: ./build/terrain_generator

Then adjust the command-line options as needed to customize the generated terrain.

Suggested Workflow:
Each time entering the program, it is recommended to first use the 'R' key to elevate viewpoint. 
This operation will provide a better perspective of the terrain, allowing user to clearly observe the features of the terrain generated. 
Once the viewpoint is adjusted, user can proceed with other actions, such as navigating the terrain or adjusting the light source.

Important Notice:
It is highly recommended to run this program directly on the lab machine. 
Testing has shown that when accessing the lab machine via SSH, the best frame rate achieved is less than 20 FPS 
(due to SSH defaulting to CPU-based parallel computation). In contrast, running the program directly on the lab 
machine maintains a stable 60 FPS even at the highest settings (leveraging NVIDIA GPU computation).



