#version 120

varying vec2 TexCoord;
varying float TerrainHeight;
varying vec3 FragNormal;
varying vec3 FragPos;

uniform sampler2D texture1; // 草地纹理
uniform sampler2D texture2; // 沙地纹理
//uniform sampler2D texture3; // 点纹理
uniform sampler2D noiseTexture; // 噪声纹理
uniform float waterLevel;   // 水平线高度
uniform bool useWaterTexture; // 是否使用水纹理
uniform vec3 ambientLight; // 环境光颜色和强度
uniform vec3 lightPos; // 光源位置
uniform vec3 viewPos; // 观察者位置
uniform float HeightDif_low;
uniform float HeightDif_high;
uniform float waterDepthMax;

void main() {
    vec4 color1 = texture2D(texture1, TexCoord); // 
    vec4 color2 = texture2D(texture2, TexCoord); // 沙地纹理颜色
    // 如果颜色接近白色，则替换为绿色
    if (color1.r > 0.9 && color1.g > 0.9 && color1.b > 0.9) {
        color1 = vec4(0.7337, 0.7494, 0.3982, 1.0); // 替换为纯绿色
    }
    //vec4 color3 = texture2D(texture3, TexCoord);
    //color1 = mix(color1, color3, 0.5);
    float noiseValue = texture2D(noiseTexture, TexCoord).r; // 获取噪声值

    vec4 terrainColor;
    // 根据高度进行纹理选择
    
    if (TerrainHeight > HeightDif_low) {
        terrainColor = color1;
    } else {
        terrainColor = color2;
    }
    //terrainColor = color2;

    // 设置阈值和深度影响因子
    float noiseThreshold = 0.5; // 阈值
    float darkenFactor = 0.1;   // 加深颜色的影响因子

    // 判断噪声值是否超过阈值
    if (noiseValue > noiseThreshold) {
        // 计算加深后的颜色
        vec3 darkenedColor = terrainColor.rgb * (1.0 - darkenFactor);
        terrainColor.rgb = mix(terrainColor.rgb, darkenedColor, (noiseValue - noiseThreshold) * 2.0);
    }

    // 计算光照
    vec3 ambient = ambientLight * terrainColor.rgb;
    vec3 norm = normalize(FragNormal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * terrainColor.rgb;

    vec3 result = ambient + diffuse;
    vec4 finalColor = vec4(result, terrainColor.a);

    // 处理水面纹理效果
    if (useWaterTexture) {
        if (TerrainHeight < waterLevel) {
            float depthFactor = (waterLevel - TerrainHeight) / waterDepthMax;
            float alpha = clamp(depthFactor + 0.2, 0.2, 0.8);
            vec4 waterColor = vec4(0.0, 0.4, 1.0, alpha);

            // 计算水面光照
            vec3 waterDiffuse = diff * waterColor.rgb;
            vec3 waterAmbient = ambientLight * waterColor.rgb;
            vec4 waterLitColor = vec4(waterAmbient + waterDiffuse, waterColor.a);

            gl_FragColor = mix(finalColor, waterLitColor, alpha);
        } else {
            discard;
        }
    } else {
        gl_FragColor = finalColor; // 使用计算的光照颜色
    }
}
