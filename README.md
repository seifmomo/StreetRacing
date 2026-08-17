# Fast & Furious: 3D Street Racing Experience

A 3D night city street racing game built entirely with OpenGL (GLUT) and C++. All objects are modeled from primitives (cubes, spheres, cylinders) with no external assets.

## Features

- **3 Camera Modes** — Third-person, first-person, and free camera (press C to switch)
- **Dynamic Lighting** — Directional moonlight, 10 street lamp point lights, and car headlights with spotlights
- **Material System** — Realistic materials for road asphalt, building concrete, car paint (metallic), glass, and rubber
- **Traffic System** — 15 AI oncoming cars with headlights, tail lights, and random colors
- **Car Model** — 20+ part sports car built from primitives: body, hood, cabin, roof, windshield, side windows, bumpers, spoiler, mirrors, neon underglow, and 4 detailed wheels with rims
- **City Environment** — Procedurally generated buildings with lit windows, street lamps, lane markings, sidewalks, and traffic barriers
- **Night Atmosphere** — Fog, dark sky, emissive glowing headlights and taillights
- **HUD** — Speedometer, camera mode display, headlight status, car color indicator

## Controls

| Key | Action |
|-----|--------|
| W / S | Accelerate / Brake |
| A / D | Steer Left / Right |
| C | Switch Camera Mode |
| L | Toggle Headlights |
| R | Change Car Color |
| Arrow Keys | Free Camera Movement |
| ENTER | Start Driving |
| ESC | Exit |

## Building

Requires MinGW g++ and FreeGLUT.

```bash
g++ -o StreetRacing.exe main.cpp -lfreeglut -lopengl32 -lglu32 -lm
```

Or run `build.bat` on Windows.

## Technical Details

- **Transformations** — Translation, rotation, scaling for all objects
- **Camera System** — Smooth lerp transitions between third-person, first-person, and free camera
- **Lighting** — 8 simultaneous light sources (GL_LIGHT0 through GL_LIGHT7)
- **Materials** — GL_AMBIENT, GL_DIFFUSE, GL_SPECULAR, GL_EMISSION, GL_SHININESS
- **Animation** — Wheel rotation, camera transitions, traffic movement, neon pulse
- **Fog** — Linear fog for depth and night atmosphere

## Project Type

Computer Graphics Final Project — demonstrates transformations, camera systems, lighting, materials, object modeling, animation, and user interaction.
