---
title: Sky, Atmosphere, and Sun Rendering Design
version: 1.0
date_created: 2026-08-10
last_updated: 2026-08-10
owner: ECE596C Terrain Generator
tags: [design, rendering, opengl, sky, atmosphere, sun]
---

# Introduction

This specification defines a small, self-contained visual upgrade for the Terrain Generator. It replaces the black background and visible light-source cube with a gradient sky, an analytic sun disk and glow, directional sunlight, lightweight atmospheric perspective, and live environment controls. The implementation must remain compatible with the existing C++23 and OpenGL 3.3 Core renderer.

## 1. Purpose & Scope

The purpose is to make the existing terrain scene read as an outdoor environment while preserving the current renderer architecture and runtime behavior.

In scope:

- A full-screen procedural gradient sky.
- A distant analytic sun disk with a soft halo.
- Directional terrain and water lighting driven by the sun direction.
- Sky colors, sun color, ambient light, and fog color derived from sun elevation.
- Lightweight exponential distance fog for atmospheric perspective.
- Dear ImGui controls for time of day, maximum solar elevation, and light intensity.
- Existing `2` and `3` keys repurposed to move time backward and forward.
- Runtime asset copying, installation, screenshot mode, and documentation updates.

Out of scope:

- Shadow maps or cascaded shadows.
- High Dynamic Range (HDR) framebuffers, bloom post-processing, or tone mapping.
- Physically based rendering (PBR), image-based lighting, or atmospheric scattering simulation.
- Skyboxes, cubemaps, cloud rendering, stars, or a moon.
- New texture assets, model assets, external libraries, or command-line options.
- Changes to terrain generation, mesh Level of Detail (LOD), water geometry, or camera behavior.

The intended audience is an implementation agent modifying this repository. The implementation must follow this specification without adding adjacent rendering features.

## 2. Definitions

- **Analytic sun**: A sun disk and halo calculated in a fragment shader from the angle between the view direction and sun direction; no mesh or texture is used.
- **Directional light**: A light represented by one normalized direction and treated as infinitely distant.
- **Atmospheric perspective**: Distance-based blending of scene color toward the horizon color.
- **Full-screen triangle**: One triangle generated from `gl_VertexID` that covers the viewport without a vertex buffer.
- **Solar elevation**: The angle of the sun above or below the horizon.
- **Maximum solar elevation**: The highest elevation reached at noon by the configured daily sun arc.
- **Time of day**: A floating-point hour in the range `[0, 24]`, where `0` and `24` are equivalent.
- **GLSL**: OpenGL Shading Language.
- **ImGui**: Dear ImGui, the existing runtime controls library.

## 3. Requirements, Constraints & Guidelines

### Functional requirements

- **REQ-001**: The renderer shall draw a procedural sky over every framebuffer pixel not covered by scene geometry.
- **REQ-002**: The sky shall use a smooth horizon-to-zenith gradient and shall not require texture assets.
- **REQ-003**: The sky shall render the sun as a distant circular disk with a wider, softer analytic halo.
- **REQ-004**: The existing white light-source cube and its render pass shall be removed.
- **REQ-005**: Terrain and water diffuse lighting shall use a normalized direction from each surface toward the sun instead of a world-space point-light position.
- **REQ-006**: Sun direction shall control the sky palette, sun color, ambient color, directional light visibility, and fog color.
- **REQ-007**: Terrain and water shall receive exponential distance fog blended toward the current horizon color.
- **REQ-008**: The ImGui panel shall expose time of day, maximum solar elevation, and light intensity controls.
- **REQ-009**: Environment control changes shall update rendering immediately without regenerating or re-uploading terrain.
- **REQ-010**: Holding key `2` shall move time backward and holding key `3` shall move time forward at `1.5` in-world hours per real second.
- **REQ-011**: Screenshot mode shall render the same default environment as interactive mode without requiring ImGui initialization.
- **REQ-012**: Day, dusk, and night shall remain visually distinct. At night the sun shall be below the horizon, directional sunlight shall approach zero, and the background shall remain a dark blue gradient rather than black.

### Default values and ranges

- **DAT-001**: Default time of day shall be `17.5` hours.
- **DAT-002**: Default maximum solar elevation shall be `55.0` degrees with an allowed UI range of `[15.0, 80.0]` degrees.
- **DAT-003**: Default light intensity shall be `1.0` with an allowed UI range of `[0.0, 2.0]`.
- **DAT-004**: The time control UI range shall be `[0.0, 24.0]` hours. Keyboard updates shall wrap values into `[0.0, 24.0)`.

### Rendering constraints

- **CON-001**: The implementation shall remain OpenGL 3.3 Core compatible and use GLSL `#version 330 core`.
- **CON-002**: The sky shall use one full-screen triangle, one empty Vertex Array Object (VAO), and one shader program. It shall not introduce a framebuffer, texture, sphere mesh, or dome mesh.
- **CON-003**: The sky pass shall run after color/depth clear and before terrain rendering. Depth testing and face culling shall be disabled only for the sky draw and restored immediately afterward.
- **CON-004**: The sun disk and halo shall be rendered inside the sky fragment shader, not as a separate draw call.
- **CON-005**: Fog shall use the existing fragment world position and camera position. It shall not require depth textures or post-processing.
- **CON-006**: Fog density shall scale with terrain extent so changing terrain width preserves a similar visual range.
- **CON-007**: Existing terrain textures, alpha-blended water, frustum culling, distance LOD, wireframe mode, screenshot output, and installation behavior shall continue to function.
- **CON-008**: No new third-party dependency or generated asset shall be added.
- **CON-009**: No unrelated refactor shall be performed.

### Visual guidelines

- **GUD-001**: The default `17.5` environment shall resemble warm golden hour: a pale warm horizon, blue upper sky, warm sunlight, and a visible low sun when the camera faces its direction.
- **GUD-002**: The sun disk edge shall be visually crisp relative to its halo but shall not be a hard-edged geometric object.
- **GUD-003**: Atmospheric fog shall improve depth separation without hiding nearby terrain. The final fog factor shall be capped at `0.72`.
- **GUD-004**: All palette interpolation shall be smooth; changing controls shall not create discrete color jumps.
- **GUD-005**: Shader output shall remain in the existing Low Dynamic Range (LDR) pipeline and avoid deliberately emitting extreme values that depend on tone mapping.

## 4. Interfaces & Data Contracts

### 4.1 CPU environment settings

`Application` shall own the following values directly or through one private aggregate:

| Field | Type | Default | Valid range | Meaning |
|---|---:|---:|---:|---|
| `timeOfDay` | `float` | `17.5` | `[0, 24]` | Current in-world hour |
| `maxSunElevationDegrees` | `float` | `55.0` | `[15, 80]` | Noon solar elevation |
| `lightIntensity` | `float` | `1.0` | `[0, 2]` | User multiplier for directional sunlight and sun appearance |

The derived sun direction shall point from a surface toward the sun and shall be computed as follows:

```text
phase = ((timeOfDay - 6.0) / 24.0) * 2*pi
maxElevation = radians(maxSunElevationDegrees)
elevation = asin(sin(phase) * sin(maxElevation))
sunDirection = normalize(
    cos(elevation) * sin(phase),
    sin(elevation),
    cos(elevation) * cos(phase)
)
```

This places sunrise at approximately `06:00`, noon at `12:00`, sunset at `18:00`, and the sun below the horizon at night. The default setting sun lies approximately in the negative world Z direction to suit the existing initial camera.

### 4.2 Derived palette

The CPU shall derive a continuous environment palette from `sunDirection.y`. Exact interpolation helpers may use `smoothstep` and linear interpolation, but shall use these anchors:

| State | Zenith RGB | Horizon RGB | Sun RGB | Ambient RGB |
|---|---|---|---|---|
| Night | `(0.015, 0.025, 0.080)` | `(0.055, 0.070, 0.130)` | `(0.0, 0.0, 0.0)` | `(0.035, 0.045, 0.080)` |
| Dusk | `(0.080, 0.180, 0.360)` | `(0.950, 0.380, 0.160)` | `(1.000, 0.520, 0.260)` | `(0.180, 0.160, 0.180)` |
| Day | `(0.120, 0.360, 0.720)` | `(0.650, 0.800, 0.950)` | `(1.000, 0.950, 0.820)` | `(0.300, 0.340, 0.400)` |

Directional light intensity shall equal the user `lightIntensity` multiplied by a smooth daylight factor that reaches zero when the sun is sufficiently below the horizon. The fog color shall equal the derived horizon color.

### 4.3 Sky shader interface

Add `shader/sky.vert` and `shader/sky.frag`.

| Uniform | GLSL type | Producer | Purpose |
|---|---|---|---|
| `uInvViewProj` | `mat4` | `Application::renderFrame` | Reconstruct world-space view direction |
| `uCameraPos` | `vec3` | `Application::renderFrame` | View-direction origin |
| `uSunDirection` | `vec3` | Environment state | Direction toward sun |
| `uZenithColor` | `vec3` | Environment palette | Upper-sky color |
| `uHorizonColor` | `vec3` | Environment palette | Horizon and fog color |
| `uSunColor` | `vec3` | Environment palette | Disk and halo color |
| `uSunVisualIntensity` | `float` | Environment state | Visual sun multiplier |

The vertex shader shall generate a full-screen triangle from `gl_VertexID`. The fragment shader shall:

1. Reconstruct a normalized world-space view direction.
2. Blend horizon and zenith colors using the view direction Y component.
3. Compute a sun disk approximately `0.45` degrees in radius with a feathered edge ending by `0.70` degrees.
4. Compute a wider soft halo from the same angular dot product.
5. Add the LDR-safe sun contribution to the gradient.

### 4.4 Terrain shader interface

Remove uniform `uLightPos` from `shader/terrain.frag` and stop providing it from C++.

Add:

| Uniform | GLSL type | Purpose |
|---|---|---|
| `uSunDirection` | `vec3` | Normalized direction from fragment toward sun |
| `uSunColor` | `vec3` | Current directional light color |
| `uSunIntensity` | `float` | Daylight-adjusted directional intensity |
| `uAmbientColor` | `vec3` | Current sky ambient color |
| `uCameraPos` | `vec3` | Distance fog origin |
| `uFogColor` | `vec3` | Current horizon color |
| `uFogDensity` | `float` | Terrain-scale-aware fog density |

Diffuse lighting shall use:

```glsl
float diffuseFactor = max(dot(normalize(FragNormal), normalize(uSunDirection)), 0.0);
vec3 lighting = uAmbientColor + uSunColor * (diffuseFactor * uSunIntensity);
vec3 litColor = surfaceColor * lighting;
```

Fog shall use:

```glsl
float distanceToCamera = length(FragPos - uCameraPos);
float fogFactor = clamp(1.0 - exp(-distanceToCamera * uFogDensity), 0.0, 0.72);
litColor = mix(litColor, uFogColor, fogFactor);
```

The same direction, palette, and fog rules shall apply to terrain and the existing water pass.

### 4.5 ImGui and keyboard interface

Add an `Environment` section to the existing control panel:

| Label | Widget | Range | Display |
|---|---|---|---|
| `Time of day` | Slider | `0.0` to `24.0` | One decimal hour |
| `Sun arc height` | Slider | `15.0` to `80.0` | Whole degrees |
| `Light intensity` | Slider | `0.0` to `2.0` | Two decimals |

The panel shall state that keys `2 / 3` change time. Existing keyboard capture rules shall remain in effect.

## 5. Acceptance Criteria

- **AC-001**: Given the default launch, when the first frame is rendered, then all background pixels shall show a non-black sky gradient.
- **AC-002**: Given a view toward the sun, when the scene is rendered, then the sun shall appear circular with a soft halo and no cube shall be present.
- **AC-003**: Given any terrain fragment, when the sun moves farther across the sky without changing terrain, then the diffuse response shall change according to one global direction and shall not depend on fragment distance from a light position.
- **AC-004**: Given distant and nearby terrain, when both are visible, then the distant terrain shall blend more strongly toward the horizon color while the nearby terrain remains readable.
- **AC-005**: Given the time control is changed, when the value crosses day, dusk, and night ranges, then sun direction, sky colors, ambient light, fog color, and directional intensity shall update immediately and continuously.
- **AC-006**: Given the sun arc height control is changed, when the time is between sunrise and sunset, then the sun elevation and lighting direction shall update without terrain regeneration.
- **AC-007**: Given the light intensity control is set to `0.0`, when rendering continues, then directional diffuse lighting and the analytic sun contribution shall disappear while ambient sky lighting remains visible.
- **AC-008**: Given key `2` or `3` is held and ImGui is not capturing the keyboard, when frames advance, then time shall move backward or forward at `1.5` hours per second and wrap at midnight.
- **AC-009**: Given `--screenshot <path>`, when the application renders and exits, then it shall produce a valid screenshot containing the default sky and terrain without initializing ImGui.
- **AC-010**: Given terrain width changes and terrain is regenerated, when the camera retains its relative position, then atmospheric fog shall retain a similar relative range.
- **AC-011**: Given the project is installed, when the installed executable starts, then both sky shader files shall be found beside the executable through the existing asset-root logic.
- **AC-012**: Given wireframe, frustum culling, distance LOD, water blending, or the F1 panel toggle is used, when the new environment rendering is active, then those existing behaviors shall continue to work.

## 6. Test Automation Strategy

- **Test Levels**: Existing CPU unit tests, OpenGL integration smoke test, screenshot failure test, and manual visual acceptance.
- **Frameworks**: Existing CTest targets and the application's built-in BMP screenshot mode. No new framework is permitted.
- **Automated build**: Configure and build the existing Release preset or equivalent build directory.
- **Automated tests**: Run all existing CTest tests. The screenshot smoke test must compile/load the new shaders and write a non-empty BMP.
- **Screenshot validation**: Capture the default frame and verify that the upper background contains multiple non-black colors and that no white cube is visible.
- **Interactive validation**: Exercise all three ImGui sliders plus keys `2` and `3`, including day, dusk, and night states.
- **Regression validation**: Toggle wireframe, frustum culling, distance LOD, and the control panel; regenerate terrain once.
- **Performance**: The feature shall add exactly one full-screen sky draw call. No numerical frame-rate target is required.
- **Coverage Requirements**: No new line-coverage threshold. Shader compilation and uniform integration are covered by the smoke executable.

## 7. Rationale & Context

The current renderer clears to black, shades terrain from a moving point-light position, and draws that light as a white cube. Those choices reveal the scene as a graphics exercise rather than an outdoor environment. A procedural full-screen sky and analytic sun produce the largest visual improvement with one draw call and no assets. Directional lighting matches an effectively distant sun. Exponential fog adds depth using data already available in the terrain shader. CPU-derived palette values keep the sky, fog, and terrain lighting synchronized while avoiding a physical atmosphere model.

The design intentionally does not introduce shadow maps, HDR, bloom, or PBR. Those are separate rendering projects and are not required to meet the requested visual goal.

## 8. Dependencies & External Integrations

### External Systems

- None.

### Third-Party Services

- None.

### Infrastructure Dependencies

- **INF-001**: Existing CMake asset synchronization and install rules must copy the complete `shader/` directory.

### Data Dependencies

- None. The sky and sun are procedural.

### Technology Platform Dependencies

- **PLT-001**: OpenGL 3.3 Core and GLSL 330 Core.
- **PLT-002**: Existing GLM matrix/vector math.
- **PLT-003**: Existing Dear ImGui integration for live controls.

### Compliance Dependencies

- None.

## 9. Examples & Edge Cases

### Default golden hour

```text
timeOfDay = 17.5
maxSunElevationDegrees = 55.0
lightIntensity = 1.0

Expected: low setting sun toward negative Z, warm horizon, blue zenith,
warm directional light, and moderate distance fog.
```

### Night

```text
timeOfDay = 0.0

Expected: sun below horizon, no visible sun disk, directional intensity near
zero, dark blue gradient, and dim blue ambient terrain. No stars or moon.
```

### Disabled sunlight

```text
lightIntensity = 0.0

Expected: analytic disk/halo and directional diffuse contribution disappear.
Sky gradient, ambient terrain visibility, water, and fog remain.
```

### Edge cases

- `timeOfDay == 24.0` shall render equivalently to `0.0`.
- A minimized zero-sized framebuffer shall retain the existing early return.
- Camera translation shall not shift the sky gradient or sun direction; only camera rotation shall change their screen position.
- Screenshot mode shall use defaults because no CLI environment controls are in scope.
- Terrain regeneration shall update fog density only as needed for the new world extent.

## 10. Validation Criteria

Implementation is compliant only when all of the following are true:

1. Requirements `REQ-001` through `REQ-012` and acceptance criteria `AC-001` through `AC-012` are satisfied.
2. No item listed as out of scope is introduced.
3. Release build succeeds without new compiler warnings.
4. All existing CTest tests pass.
5. Default screenshot visually shows a gradient sky, atmospheric depth, and no cube.
6. Interactive controls update continuously without terrain regeneration or errors.
7. Installed runtime locates `sky.vert` and `sky.frag` through existing asset handling.
8. Git working tree changes are divided into these commits:
   - Specification only.
   - Gradient sky and atmospheric perspective.
   - Analytic sun and directional lighting, including cube removal.
   - Environment controls and documentation.
9. The pre-existing untracked `imgui.ini` file remains outside all commits.

## 11. Related Specifications / Further Reading

- [Project README](../README.md)
- [Modern C++ refactoring notes](../REFACTORING.md)
