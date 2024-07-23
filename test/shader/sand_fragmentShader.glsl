#version 120
varying vec2 TexCoord;
varying float Height;
uniform sampler2D texture1; // 草地纹理
uniform sampler2D texture2; // 沙地纹理
uniform sampler2D texture3; // 水纹理
uniform float waterLevel;   // 水平线高度

void main() {
    vec4 color1 = texture2D(texture2, TexCoord);
    vec4 color2 = texture2D(texture1, TexCoord);
    vec4 color3 = texture2D(texture3, TexCoord);

    vec4 terrainColor;
    float factor = clamp((Height - 5.0) / 10.0, 0.0, 1.0);
    terrainColor = mix(color1, color2, factor);

    if (Height < waterLevel) {
        float depth = waterLevel - Height;
        float alpha = clamp(depth / 3.0, 0.0, 1.0); // 调整这个值以改变透明度范围
        color3.a = 0.3; // 设置水的透明度
        gl_FragColor = mix(terrainColor, color3, color3.a); // 混合地形和水
    } else {
        gl_FragColor = terrainColor;
    }
}
