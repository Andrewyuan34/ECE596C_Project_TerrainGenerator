#version 120
attribute vec3 aPos;
attribute vec2 aTexCoord;
attribute float aHeight; // 地形高度
varying vec2 TexCoord;
varying float TerrainHeight;

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    TerrainHeight = aHeight; // 传递地形高度
}
