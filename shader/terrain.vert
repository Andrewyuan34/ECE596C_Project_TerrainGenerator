#version 330 core

// Vertex shader for terrain rendering (OpenGL 3.3 Core)

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aHeight;

uniform mat4 uViewProj;   // projection * view (model is the identity matrix)
uniform float uWaterLevel;
uniform bool uDrawWater;

out vec2 TexCoord;
out float TerrainHeight;
out vec3 FragNormal;
out vec3 FragPos;

void main() {
    vec3 position = aPos;
    if (uDrawWater)
        position.y = uWaterLevel;

    gl_Position  = uViewProj * vec4(position, 1.0);
    TexCoord     = aTexCoord;
    TerrainHeight = aHeight;
    FragNormal   = uDrawWater ? vec3(0.0, 1.0, 0.0) : aNormal;
    FragPos      = position;
}
