#version 120

// Vertex shader for terrain rendering

// Input attributes
attribute vec3 aPos;        // Vertex position
attribute vec3 aNormal;     // Vertex normal
attribute vec2 aTexCoord;   // Texture coordinates
attribute float aHeight;    // Terrain height attribute

// Output varyings
varying vec2 TexCoord;      // Texture coordinates for fragment shader
varying float TerrainHeight; // Terrain height to pass to fragment shader
varying vec3 FragNormal;    // Normal vector to pass to fragment shader
varying vec3 FragPos;       // Vertex position in world space to pass to fragment shader

void main() {
    // Calculate vertex position in clip space
    gl_Position = gl_ModelViewProjectionMatrix * vec4(aPos, 1.0);

    // Pass varying values to fragment shader
    TexCoord = aTexCoord;       // Pass texture coordinates
    TerrainHeight = aHeight;    // Pass terrain height
    FragNormal = aNormal;       // Pass vertex normal
    FragPos = vec3(gl_ModelViewMatrix * vec4(aPos, 1.0)); // Calculate and pass vertex position in world space
}

