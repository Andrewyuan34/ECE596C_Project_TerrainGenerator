# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.29

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /home/frodo/public/ugls_lab-10.4.2/packages/cmake-3.29.2/bin/cmake

# The command to remove a file.
RM = /home/frodo/public/ugls_lab-10.4.2/packages/cmake-3.29.2/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/yiboy/Desktop/cpp_project-yibo_yuan

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake

# Include any dependencies generated for this target.
include CMakeFiles/terrain_generator.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/terrain_generator.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/terrain_generator.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/terrain_generator.dir/flags.make

CMakeFiles/terrain_generator.dir/src/main.cpp.o: CMakeFiles/terrain_generator.dir/flags.make
CMakeFiles/terrain_generator.dir/src/main.cpp.o: /home/yiboy/Desktop/cpp_project-yibo_yuan/src/main.cpp
CMakeFiles/terrain_generator.dir/src/main.cpp.o: CMakeFiles/terrain_generator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/terrain_generator.dir/src/main.cpp.o"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/terrain_generator.dir/src/main.cpp.o -MF CMakeFiles/terrain_generator.dir/src/main.cpp.o.d -o CMakeFiles/terrain_generator.dir/src/main.cpp.o -c /home/yiboy/Desktop/cpp_project-yibo_yuan/src/main.cpp

CMakeFiles/terrain_generator.dir/src/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/terrain_generator.dir/src/main.cpp.i"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yiboy/Desktop/cpp_project-yibo_yuan/src/main.cpp > CMakeFiles/terrain_generator.dir/src/main.cpp.i

CMakeFiles/terrain_generator.dir/src/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/terrain_generator.dir/src/main.cpp.s"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yiboy/Desktop/cpp_project-yibo_yuan/src/main.cpp -o CMakeFiles/terrain_generator.dir/src/main.cpp.s

CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o: CMakeFiles/terrain_generator.dir/flags.make
CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o: /home/yiboy/Desktop/cpp_project-yibo_yuan/src/command_line_parser.cpp
CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o: CMakeFiles/terrain_generator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o -MF CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o.d -o CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o -c /home/yiboy/Desktop/cpp_project-yibo_yuan/src/command_line_parser.cpp

CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.i"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yiboy/Desktop/cpp_project-yibo_yuan/src/command_line_parser.cpp > CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.i

CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.s"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yiboy/Desktop/cpp_project-yibo_yuan/src/command_line_parser.cpp -o CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.s

CMakeFiles/terrain_generator.dir/src/camera.cpp.o: CMakeFiles/terrain_generator.dir/flags.make
CMakeFiles/terrain_generator.dir/src/camera.cpp.o: /home/yiboy/Desktop/cpp_project-yibo_yuan/src/camera.cpp
CMakeFiles/terrain_generator.dir/src/camera.cpp.o: CMakeFiles/terrain_generator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/terrain_generator.dir/src/camera.cpp.o"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/terrain_generator.dir/src/camera.cpp.o -MF CMakeFiles/terrain_generator.dir/src/camera.cpp.o.d -o CMakeFiles/terrain_generator.dir/src/camera.cpp.o -c /home/yiboy/Desktop/cpp_project-yibo_yuan/src/camera.cpp

CMakeFiles/terrain_generator.dir/src/camera.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/terrain_generator.dir/src/camera.cpp.i"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yiboy/Desktop/cpp_project-yibo_yuan/src/camera.cpp > CMakeFiles/terrain_generator.dir/src/camera.cpp.i

CMakeFiles/terrain_generator.dir/src/camera.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/terrain_generator.dir/src/camera.cpp.s"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yiboy/Desktop/cpp_project-yibo_yuan/src/camera.cpp -o CMakeFiles/terrain_generator.dir/src/camera.cpp.s

CMakeFiles/terrain_generator.dir/src/lighting.cpp.o: CMakeFiles/terrain_generator.dir/flags.make
CMakeFiles/terrain_generator.dir/src/lighting.cpp.o: /home/yiboy/Desktop/cpp_project-yibo_yuan/src/lighting.cpp
CMakeFiles/terrain_generator.dir/src/lighting.cpp.o: CMakeFiles/terrain_generator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/terrain_generator.dir/src/lighting.cpp.o"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/terrain_generator.dir/src/lighting.cpp.o -MF CMakeFiles/terrain_generator.dir/src/lighting.cpp.o.d -o CMakeFiles/terrain_generator.dir/src/lighting.cpp.o -c /home/yiboy/Desktop/cpp_project-yibo_yuan/src/lighting.cpp

CMakeFiles/terrain_generator.dir/src/lighting.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/terrain_generator.dir/src/lighting.cpp.i"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yiboy/Desktop/cpp_project-yibo_yuan/src/lighting.cpp > CMakeFiles/terrain_generator.dir/src/lighting.cpp.i

CMakeFiles/terrain_generator.dir/src/lighting.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/terrain_generator.dir/src/lighting.cpp.s"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yiboy/Desktop/cpp_project-yibo_yuan/src/lighting.cpp -o CMakeFiles/terrain_generator.dir/src/lighting.cpp.s

CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o: CMakeFiles/terrain_generator.dir/flags.make
CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o: /home/yiboy/Desktop/cpp_project-yibo_yuan/src/TerrainGenerate.cpp
CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o: CMakeFiles/terrain_generator.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o -MF CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o.d -o CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o -c /home/yiboy/Desktop/cpp_project-yibo_yuan/src/TerrainGenerate.cpp

CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Preprocessing CXX source to CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.i"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/yiboy/Desktop/cpp_project-yibo_yuan/src/TerrainGenerate.cpp > CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.i

CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green "Compiling CXX source to assembly CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.s"
	/home/frodo/public/ugls_lab-10.4.2/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/yiboy/Desktop/cpp_project-yibo_yuan/src/TerrainGenerate.cpp -o CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.s

# Object files for target terrain_generator
terrain_generator_OBJECTS = \
"CMakeFiles/terrain_generator.dir/src/main.cpp.o" \
"CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o" \
"CMakeFiles/terrain_generator.dir/src/camera.cpp.o" \
"CMakeFiles/terrain_generator.dir/src/lighting.cpp.o" \
"CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o"

# External object files for target terrain_generator
terrain_generator_EXTERNAL_OBJECTS =

terrain_generator: CMakeFiles/terrain_generator.dir/src/main.cpp.o
terrain_generator: CMakeFiles/terrain_generator.dir/src/command_line_parser.cpp.o
terrain_generator: CMakeFiles/terrain_generator.dir/src/camera.cpp.o
terrain_generator: CMakeFiles/terrain_generator.dir/src/lighting.cpp.o
terrain_generator: CMakeFiles/terrain_generator.dir/src/TerrainGenerate.cpp.o
terrain_generator: CMakeFiles/terrain_generator.dir/build.make
terrain_generator: /home/frodo/public/ugls_lab-10.4.2/packages/boost/lib/libboost_program_options.so
terrain_generator: /usr/lib64/libGL.so
terrain_generator: /usr/lib64/libGLU.so
terrain_generator: /usr/lib64/libGLEW.so
terrain_generator: /usr/lib64/libglut.so
terrain_generator: /usr/lib64/libXi.so
terrain_generator: CMakeFiles/terrain_generator.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --green --bold --progress-dir=/home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Linking CXX executable terrain_generator"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/terrain_generator.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/terrain_generator.dir/build: terrain_generator
.PHONY : CMakeFiles/terrain_generator.dir/build

CMakeFiles/terrain_generator.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/terrain_generator.dir/cmake_clean.cmake
.PHONY : CMakeFiles/terrain_generator.dir/clean

CMakeFiles/terrain_generator.dir/depend:
	cd /home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/yiboy/Desktop/cpp_project-yibo_yuan /home/yiboy/Desktop/cpp_project-yibo_yuan /home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake /home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake /home/yiboy/Desktop/cpp_project-yibo_yuan/tmp_cmake/CMakeFiles/terrain_generator.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/terrain_generator.dir/depend

