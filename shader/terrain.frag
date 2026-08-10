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
uniform vec3  uSunDirection;
uniform vec3  uSunColor;
uniform float uSunIntensity;
uniform vec3  uAmbientColor;

// Lightweight atmospheric perspective
uniform vec3  uCameraPos;
uniform vec3  uFogColor;
uniform float uFogDensity;

// Height range for terrain texture blending
uniform float uHeightDifLow;
uniform float uHeightDifHigh;

vec3 applyDistanceFog(vec3 color) {
    float distanceToCamera = length(FragPos - uCameraPos);
    float fogFactor = clamp(1.0 - exp(-distanceToCamera * uFogDensity), 0.0, 0.72);
    return mix(color, uFogColor, fogFactor);
}

void main() {
    vec4 grass = texture(uGrassTexture, TexCoord);
    vec4 sand  = texture(uSandTexture,  TexCoord);

    // Blend sand -> grass based on terrain height
    float factor = clamp((TerrainHeight - uHeightDifLow) / uHeightDifHigh, 0.0, 1.0);
    vec4 terrainColor = mix(sand, grass, factor);

    // Sky ambient + directional sunlight
    vec3 ambient  = uAmbientColor * terrainColor.rgb;
    vec3 norm     = normalize(FragNormal);
    vec3 lightDir = normalize(uSunDirection);
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = uSunColor * (diff * uSunIntensity) * terrainColor.rgb;

    vec4 finalColor = vec4(ambient + diffuse, terrainColor.a);

    if (uDrawWater) {
        if (TerrainHeight < uWaterLevel) {
            // Depth-based transparency
            float depthFactor = (uWaterLevel - TerrainHeight) / uWaterDepthMax;
            float alpha = clamp(depthFactor + 0.2, 0.2, 0.8);

            vec4 waterColor   = vec4(0.0, 0.4, 1.0, alpha);
            vec3 waterDiffuse = uSunColor * (diff * uSunIntensity) * waterColor.rgb;
            vec3 waterAmbient = uAmbientColor * waterColor.rgb;
            vec4 waterLit     = vec4(waterAmbient + waterDiffuse, waterColor.a);

            // The terrain is already in the framebuffer. Output the water
            // layer once and let GL_SRC_ALPHA blending composite it.
            FragColor = vec4(applyDistanceFog(waterLit.rgb), waterLit.a);
        } else {
            discard; // Water plane fragments above the terrain are invisible
        }
    } else {
        FragColor = vec4(applyDistanceFog(finalColor.rgb), finalColor.a);
    }
}
