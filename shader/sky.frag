#version 330 core

in vec2 NdcPosition;

out vec4 FragColor;

uniform mat4 uInvViewProj;
uniform vec3 uCameraPos;
uniform vec3 uZenithColor;
uniform vec3 uHorizonColor;

void main() {
    vec4 farWorld = uInvViewProj * vec4(NdcPosition, 1.0, 1.0);
    vec3 worldPosition = farWorld.xyz / farWorld.w;
    vec3 viewDirection = normalize(worldPosition - uCameraPos);

    float height = smoothstep(-0.08, 0.72, viewDirection.y);
    float gradient = pow(height, 0.75);
    vec3 skyColor = mix(uHorizonColor, uZenithColor, gradient);
    float lowerHemisphereLight = mix(
        0.55, 1.0, smoothstep(-0.70, -0.02, viewDirection.y));
    skyColor *= lowerHemisphereLight;
    FragColor = vec4(skyColor, 1.0);
}
