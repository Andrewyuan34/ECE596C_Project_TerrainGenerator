#version 120

// Varying variables passed from the vertex shader
varying vec2 TexCoord;
varying float TerrainHeight;
varying vec3 FragNormal;
varying vec3 FragPos;

// Textures for terrain and water
uniform sampler2D texture1; // Texture for grassland
uniform sampler2D texture2; // Texture for sandy areas

// Water parameters
uniform float waterLevel;    // Height at which water starts
uniform bool useWaterTexture; // Flag indicating whether to use water texture

// Lighting parameters
uniform vec3 ambientLight; // Ambient light color and intensity
uniform vec3 lightPos;     // Position of the light source

// Height range for terrain texture blending
uniform float HeightDif_low;
uniform float HeightDif_high;

// Maximum depth of water for depth-based transparency
uniform float waterDepthMax;

void main() {
    // Sample textures based on texture coordinates
    vec4 color1 = texture2D(texture1, TexCoord); // Grassland texture
    vec4 color2 = texture2D(texture2, TexCoord); // Sandy area texture

    // Determine terrain color based on height
    vec4 terrainColor;
    float factor = clamp((TerrainHeight - HeightDif_low) / HeightDif_high, 0.0, 1.0);
    terrainColor = mix(color2, color1, factor);

    // Calculate ambient light contribution
    vec3 ambient = ambientLight * terrainColor.rgb;

    // Calculate diffuse light contribution
    vec3 norm = normalize(FragNormal);       // Normal vector
    vec3 lightDir = normalize(lightPos - FragPos); // Light direction
    float diff = max(dot(norm, lightDir), 0.0);     // Diffuse component
    vec3 diffuse = diff * terrainColor.rgb;

    // Final computed color with lighting
    vec3 result = ambient + diffuse;
    vec4 finalColor = vec4(result, terrainColor.a);

    // Apply water surface texture effect if enabled
    if (useWaterTexture) {
        // Check if current fragment is below water level
        if (TerrainHeight < waterLevel) {
            // Calculate depth factor for transparency
            float depthFactor = (waterLevel - TerrainHeight) / waterDepthMax;
            float alpha = clamp(depthFactor + 0.2, 0.2, 0.8); // Adjust alpha for transparency effect

            // Water color with ambient and diffuse lighting
            vec4 waterColor = vec4(0.0, 0.4, 1.0, alpha);

            vec3 waterDiffuse = diff * waterColor.rgb;      // Water diffuse lighting
            vec3 waterAmbient = ambientLight * waterColor.rgb; // Water ambient lighting
            vec4 waterLitColor = vec4(waterAmbient + waterDiffuse, waterColor.a); // Combined water color

            // Mix final color between terrain and water based on alpha
            gl_FragColor = mix(finalColor, waterLitColor, alpha);
        } else {
            discard; // Discard fragments above water level when water texture is enabled
        }
    } else {
        gl_FragColor = finalColor; // Use computed color without water effect
    }
}
