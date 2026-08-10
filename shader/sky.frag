#version 330 core

in vec2 NdcPosition;

out vec4 FragColor;

uniform mat4 uInvViewProj;
uniform vec3 uCameraPos;
uniform vec3 uZenithColor;
uniform vec3 uHorizonColor;
uniform vec3 uSunDirection;
uniform vec3 uSunColor;
uniform float uSunVisualIntensity;

void main() {
    vec4 farWorld = uInvViewProj * vec4(NdcPosition, 1.0, 1.0);
    vec3 worldPosition = farWorld.xyz / farWorld.w;
    vec3 viewDirection = normalize(worldPosition - uCameraPos);

    float height = smoothstep(-0.15, 0.25, viewDirection.y);
    float gradient = pow(height, 0.75);
    vec3 skyColor = mix(uHorizonColor, uZenithColor, gradient);
    float lowerHemisphereLight = mix(
        0.55, 1.0, smoothstep(-0.70, -0.02, viewDirection.y));
    skyColor *= lowerHemisphereLight;

    float viewSun = max(dot(viewDirection, normalize(uSunDirection)), 0.0);
    float disk = smoothstep(
        cos(radians(0.70)), cos(radians(0.45)), viewSun);
    float halo = pow(viewSun, 256.0);
    float horizonVisibility = smoothstep(-0.01, 0.02, uSunDirection.y);
    float sunBlend = clamp(
        (0.85 * disk + 0.35 * halo) * uSunVisualIntensity * horizonVisibility,
        0.0, 0.92);
    skyColor = mix(skyColor, uSunColor, sunBlend);

    FragColor = vec4(skyColor, 1.0);
}
