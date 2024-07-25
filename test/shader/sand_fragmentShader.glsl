#version 120

varying vec2 TexCoord;
varying float TerrainHeight;
uniform sampler2D texture1; // 草地纹理
uniform sampler2D texture2; // 沙地纹理
uniform float waterLevel;   // 水平线高度
uniform bool useWaterTexture; // 是否使用水纹理
uniform vec3 ambientLight; // 环境光颜色和强度

void main() {
    vec4 color1 = texture2D(texture1, TexCoord);
    vec4 color2 = texture2D(texture2, TexCoord);

    vec4 terrainColor;
    float factor = clamp((TerrainHeight - 5.0) / 10.0, 0.0, 1.0);
    terrainColor = mix(color1, color2, factor);

    if (useWaterTexture) {
        if (TerrainHeight < waterLevel) {
            float depth = waterLevel - TerrainHeight;
            float alpha = clamp(depth / 5.0, 0.0, 1.0); // 调整这个值以改变透明度范围
            vec4 waterColor = vec4(0.0, 0.4, 1.0, 0.3); // 设置水的颜色 (蓝色) 和透明度
            waterColor.a = alpha * 0.3; // 调整水的透明度
            // 应用环境光
            vec3 ambientWater = ambientLight * waterColor.rgb;
            gl_FragColor = mix(vec4(ambientWater, waterColor.a), terrainColor, waterColor.a); // 混合地形和水
        } else {
            discard;
        }
    } else {
        // 仅应用环境光照
        vec3 ambient = ambientLight * terrainColor.rgb;
        gl_FragColor = vec4(ambient, terrainColor.a);
    }
}
