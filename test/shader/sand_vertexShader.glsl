#version 120
attribute vec3 aPos;
attribute vec2 aTexCoord;
varying vec2 TexCoord;
varying float Height;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    Height = aPos.y;
}
