#version 330 core

// Fragment shader for the light-source cube (OpenGL 3.3 Core)

uniform vec3 uLightColor;

out vec4 FragColor;

void main() {
    FragColor = vec4(uLightColor, 1.0);
}
