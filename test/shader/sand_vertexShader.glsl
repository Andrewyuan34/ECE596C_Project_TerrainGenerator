#version 120

attribute vec3 aPos;
attribute vec3 aNormal; // 新增法线属性
attribute vec2 aTexCoord;
attribute float aHeight; // 地形高度

varying vec2 TexCoord;
varying float TerrainHeight;
varying vec3 FragNormal; // 传递法线到片段着色器
varying vec3 FragPos; // 传递片段位置到片段着色器

void main() {
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPos, 1.0);
    TexCoord = aTexCoord;
    TerrainHeight = aHeight; // 传递地形高度
    FragNormal = aNormal; // 传递法线
    FragPos = vec3(gl_ModelViewMatrix * vec4(aPos, 1.0)); // 计算并传递片段位置
}
