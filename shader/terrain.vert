#version 330 core

// Vertex shader for terrain rendering (OpenGL 3.3 Core)

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aHeight;

uniform mat4 uViewProj;   // projection * view (model is the identity matrix)

out vec2 TexCoord;
out float TerrainHeight;
out vec3 FragNormal;
out vec3 FragPos;

void main() {
    gl_Position  = uViewProj * vec4(aPos, 1.0);
    TexCoord     = aTexCoord;
    TerrainHeight = aHeight;
    FragNormal   = aNormal;
    FragPos      = aPos;
}
