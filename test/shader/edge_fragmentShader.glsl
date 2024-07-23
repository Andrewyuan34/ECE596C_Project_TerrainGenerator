#version 120

varying vec2 vTexCoord;
uniform sampler2D texture1;

const float offset = 1.0 / 300.0; // 偏移量，用于卷积核

void main()
{
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // 左上
        vec2( 0.0f,    offset), // 上
        vec2( offset,  offset), // 右上
        vec2(-offset,  0.0f),   // 左
        vec2( 0.0f,    0.0f),   // 中
        vec2( offset,  0.0f),   // 右
        vec2(-offset, -offset), // 左下
        vec2( 0.0f,   -offset), // 下
        vec2( offset, -offset)  // 右下
    );

    float kernel[9] = float[](
        1.0,  1.0, 1.0,
        1.0, -8.0, 1.0,
        1.0,  1.0, 1.0
    );

    float edgeDetection = 0.0;
    for (int i = 0; i < 9; i++)
    {
        edgeDetection += texture2D(texture1, vTexCoord + offsets[i]).r * kernel[i];
    }

    // 添加噪声进行扭曲
    float noise = (fract(sin(dot(vTexCoord, vec2(12.9898, 78.233))) * 43758.5453) - 0.5) * 0.1;
    edgeDetection += noise;

    gl_FragColor = vec4(vec3(edgeDetection), 1.0);
}

