#version 120
varying vec2 TexCoord;
varying float TerrainHeight;
uniform sampler2D texture1; // 草地纹理
uniform sampler2D texture2; // 沙地纹理
uniform float waterLevel;   // 水平线高度
uniform bool useWaterTexture; // 是否使用水纹理

void main() {
    vec4 color1 = texture2D(texture1, TexCoord);
    vec4 color2 = texture2D(texture2, TexCoord);

    vec4 terrainColor;
    float factor = clamp((TerrainHeight - 5.0) / 10.0, 0.0, 1.0); //genType clamp(genType x, genType minVal, genType maxVal); 大于最大则返回最大，小于最小则返回最小，其余返回自身
    terrainColor = mix(color1, color2, factor); //genType mix(genType x, genType y, genType a);

    if (useWaterTexture) {
        if (TerrainHeight < waterLevel) {
            float depth = waterLevel - TerrainHeight;
            float alpha = clamp(depth / 5.0, 0.0, 1.0); // 调整这个值以改变透明度范围
            vec4 waterColor = vec4(0.0, 0.4, 1.0, 0.3); // 设置水的颜色 (蓝色) 和透明度
            waterColor.a = alpha * 0.3; // 调整水的透明度
            gl_FragColor = mix(terrainColor, waterColor, waterColor.a); // 混合地形和水

        } else {
            discard;
        }
    } else {
        gl_FragColor = terrainColor;
    }
}
