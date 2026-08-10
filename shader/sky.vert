#version 330 core

// One full-screen triangle generated without a vertex buffer.

out vec2 NdcPosition;

const vec2 kPositions[3] = vec2[](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);

void main() {
    NdcPosition = kPositions[gl_VertexID];
    gl_Position = vec4(NdcPosition, 0.0, 1.0);
}
