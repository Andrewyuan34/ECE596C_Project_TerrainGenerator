#version 330 core

// Fragment shader for terrain rendering (OpenGL 3.3 Core)

in vec2 TexCoord;
in float TerrainHeight;
in vec3 FragNormal;
in vec3 FragPos;

out vec4 FragColor;

// Textures
uniform sampler2D uGrassTexture;
uniform sampler2D uSandTexture;

// Water parameters
uniform float uWaterLevel;
uniform bool  uDrawWater;
uniform float uWaterDepthMax;

// Lighting parameters
uniform vec3 uAmbientLight;
uniform vec3 uLightPos;

// Height range for terrain texture blending
uniform float uHeightDifLow;
uniform float uHeightDifHigh;

void main() {
    vec4 grass = texture(uGrassTexture, TexCoord);
    vec4 sand  = texture(uSandTexture,  TexCoord);

    // Blend sand -> grass based on terrain height
    float factor = clamp((TerrainHeight - uHeightDifLow) / uHeightDifHigh, 0.0, 1.0);
    vec4 terrainColor = mix(sand, grass, factor);

    // Ambient + diffuse lighting
    vec3 ambient  = uAmbientLight * terrainColor.rgb;
    vec3 norm     = normalize(FragNormal);
    vec3 lightDir = normalize(uLightPos - FragPos);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = diff * terrainColor.rgb;

    vec4 finalColor = vec4(ambient + diffuse, terrainColor.a);

    if (uDrawWater) {
        if (TerrainHeight < uWaterLevel) {
            // Depth-based transparency
            float depthFactor = (uWaterLevel - TerrainHeight) / uWaterDepthMax;
            float alpha = clamp(depthFactor + 0.2, 0.2, 0.8);

            vec4 waterColor   = vec4(0.0, 0.4, 1.0, alpha);
            vec3 waterDiffuse = diff * waterColor.rgb;
            vec3 waterAmbient = uAmbientLight * waterColor.rgb;
            vec4 waterLit     = vec4(waterAmbient + waterDiffuse, waterColor.a);

            FragColor = mix(finalColor, waterLit, alpha);
        } else {
            discard; // Water plane fragments above the terrain are invisible
        }
    } else {
        FragColor = finalColor;
    }
}
