#version 120

attribute vec3 aPos;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main()
{
    vTexCoord = aTexCoord;
    gl_Position = projectionMatrix * viewMatrix * vec4(aPos, 1.0);
}

