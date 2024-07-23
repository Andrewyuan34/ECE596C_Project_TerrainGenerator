#version 120
varying vec2 TexCoord;
varying float Height;
uniform sampler2D waterTexture; // 水纹理
uniform float waterLevel;       // 水平线高度

void main() {
    vec4 waterColor = texture2D(waterTexture, TexCoord);

    float depth = waterLevel - Height;
    float alpha = clamp(depth / 5.0, 0.0, 1.0); // 调整这个值以改变透明度范围
    waterColor.a = alpha; // 设置水的透明度

    gl_FragColor = waterColor;
}
