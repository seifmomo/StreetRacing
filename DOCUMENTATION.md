# FAST & FURIOUS: 3D STREET RACING EXPERIENCE
## Computer Graphics Final Project Documentation

---

## 1. Project Overview

**Title:** Fast & Furious: 3D Street Racing Experience
**Platform:** OpenGL (GLUT) + C++ (MinGW)
**Type:** Real-time 3D graphics application

### Description
A 3D street racing game set in a modern city at night. The player controls a sports car driving down a city street with buildings, street lamps, traffic barriers, and AI traffic. The game demonstrates all core computer graphics concepts: transformations, camera systems, lighting, material properties, object modeling from primitives, animation, and user interaction.

### Key Features
- Controllable sports car built entirely from OpenGL primitives
- Night city environment with procedural buildings and street lamps
- Three camera modes with smooth transitions
- Dynamic lighting (moonlight, street lamps, car headlights)
- Material system for different surface types
- HUD with speed, camera mode, and controls display
- 8 car color options
- AI traffic cars

---

## 2. Graphics Concepts Used

### 2.1 Transformations
| Transformation | Implementation |
|---------------|---------------|
| **Translation** | `glTranslatef()` for positioning car, buildings, lamps, road segments |
| **Rotation** | `glRotatef()` for car heading, wheel steering, wheel spinning |
| **Scaling** | `glScalef()` for resizing car body parts, buildings, barriers |

**Specific examples:**
- Car movement: `glTranslatef(carX, carY, carZ)` + `glRotatef(carHeading, 0, 1, 0)`
- Wheel spin: `glRotatef(wheelRotation, 1, 0, 0)` inside `drawWheel()`
- Building windows: translated to grid positions on building faces
- Road segments: scaled boxes for lane markings and sidewalks

### 2.2 Camera System
Three camera modes implemented with smooth interpolation (lerp):

| Mode | Description | Implementation |
|------|-------------|---------------|
| **Third Person** | Behind and above car | Camera positioned at distance behind car, looking ahead |
| **First Person** | Driver's seat view | Camera at car position, looking forward along heading |
| **Free Camera** | Fly around scene | Arrow keys move camera independently of car |

Camera transitions use smooth-step interpolation over 0.5 seconds to avoid jarring jumps.

### 2.3 Lighting and Materials

**Light Sources:**
| Light | Type | Purpose |
|-------|------|---------|
| GL_LIGHT0 | Directional | Moonlight (dim blue, w=0) |
| GL_LIGHT1-5 | Point | Street lamp illumination with attenuation |
| GL_LIGHT6-7 | Spot | Car headlights that follow the car |

**Material Properties:**
| Object | Ambient | Diffuse | Specular | Shininess |
|--------|---------|---------|----------|-----------|
| Car body | 35% of diffuse | Color-based | Medium | 60 |
| Road | 35% of diffuse | Dark gray | Very low | 5 |
| Glass windows | 35% of diffuse | Blue tint | High | 120 |
| Chrome/metal | 35% of diffuse | Silver | High | 80 |
| Building walls | 35% of diffuse | Varies | Low | 15 |

**Emissive materials** used for: headlights, tail lights, lamp fixtures, window glow, neon underglow.

**Atmospheric fog** (GL_LINEAR) creates night depth effect.

### 2.4 Object Modeling
All objects built from primitives:

| Object | Primitives Used |
|--------|----------------|
| Sports car | 20+ boxes, 8 cylinders, 4 spheres |
| Buildings | Boxes (structure), boxes (windows) |
| Street lamps | Cylinders (pole, arm), box (fixture) |
| Road | Box (asphalt), boxes (lane markings) |
| Traffic barriers | Boxes with alternating colors |
| Traffic cars | Boxes, cylinders (wheels), boxes (windows) |

### 2.5 Animation
- **Wheel rotation:** `wheelRotation += carSpeed * 60 * dt` (continuous spin)
- **Steering wheels:** Front wheels rotate around Y based on steer input
- **Camera transitions:** Smooth lerp between camera positions
- **Traffic movement:** Cars move at varying speeds, respawn when off-screen
- **Headlight tracking:** Spot lights update position/direction each frame
- **Neon underglow pulse:** Emissive intensity varies with sin(time)

### 2.6 User Interaction
| Key | Action |
|-----|--------|
| W/S | Accelerate / Brake |
| A/D | Steer left / right |
| C | Cycle camera mode (Third → First → Free) |
| L | Toggle headlights on/off |
| R | Cycle car color (8 options) |
| Arrow keys | Free camera movement |
| Enter | Start game |
| Escape | Exit |

---

## 3. Technical Implementation

### Architecture
Single-file C++ application (~1100 lines) organized into clearly labeled sections:

```
main.cpp
├── Constants & Global State
├── Utility Functions (lerp, clamp)
├── Material System (setMaterial, setEmissive)
├── Drawing Primitives (drawBox, drawCylinder, drawSphere)
├── Object: Sports Car (drawWheel, drawSportsCar)
├── Object: Road (drawRoad)
├── Object: Buildings (initBuildings, drawBuilding)
├── Object: Street Lamps (initLamps, drawLampPost)
├── Object: Traffic Barriers (drawBarrier)
├── Object: Traffic Cars (initTraffic, drawTrafficCar)
├── Lighting Setup (setupLighting, updateHeadlights)
├── Game Logic (resetGame, updateGame)
├── Camera System (switchCamera, updateCamera)
├── HUD (drawText, drawHUD)
├── Display, Input, Init, Main
```

### Build System
- **Compiler:** MinGW g++ (GCC 6.3.0+)
- **Libraries:** FreeGLUT, OpenGL32, GLU32
- **Build command:** `g++ -o StreetRacing.exe main.cpp -lfreeglut -lopengl32 -lglu32 -lm`
- **IDE support:** Code::Blocks project file included

### Performance Considerations
- Static geometry (buildings, road) rendered every frame
- Dynamic objects (car, traffic) updated with delta-time physics
- Fog culling: distant objects naturally fade
- Particle count limited (no heavy particle system)
- Street lamp point lights limited to 5 for performance

---

## 4. Challenges and Solutions

### Challenge 1: Night Scene Without Textures
**Problem:** Without texture support, the night city scene would look flat and uninteresting.
**Solution:** Used emissive materials for lights, atmospheric fog for depth, varied building colors, and window glow effects to create visual richness.

### Challenge 2: Smooth Camera Transitions
**Problem:** Switching cameras instantly would cause disorientation.
**Solution:** Implemented smooth-step interpolation that blends between old and new camera positions over 0.5 seconds.

### Challenge 3: Realistic Car Handling
**Problem:** Making the car feel responsive yet realistic.
**Solution:** Applied steering rate that decreases at high speed, friction-based deceleration, and road boundary clamping with speed penalty.

### Challenge 4: Dynamic Headlight Following
**Problem:** Headlights must follow the car's position and direction.
**Solution:** Recalculate spotlight position and direction every frame based on car's current heading using trigonometry.

### Challenge 5: Multiple Light Sources
**Problem:** GL limited to 8 lights, need moon + 5 lamps + 2 headlights.
**Solution:** Used GL_LIGHT0 for directional moon, GL_LIGHT1-5 for static lamp points, GL_LIGHT6-7 for dynamic headlights.

---

## 5. Team Contribution

| Member | Role | Contributions |
|--------|------|---------------|
| [Member 1] | Project Lead | Project planning, code architecture, car modeling |
| [Member 2] | Graphics | Lighting setup, material system, fog implementation |
| [Member 3] | Environment | Building generation, road design, street lamps |
| [Member 4] | Interaction | Camera system, keyboard input, HUD overlay |
| [Member 5] | Gameplay | Traffic AI, car physics, animation, testing |

*Note: All members participated in code review, documentation, and presentation preparation.*

---

## 6. How to Run

### Prerequisites
- MinGW (g++) with OpenGL support
- FreeGLUT installed

### Steps
1. Open Command Prompt in the project folder
2. Run: `build.bat`
3. Or open `StreetRacing.cbp` in Code::Blocks and press Build & Run
4. The game window opens showing the menu
5. Press **Enter** to start driving

### Controls Quick Reference
```
W/S       - Accelerate / Brake
A/D       - Steer Left / Right
C         - Switch Camera Mode
L         - Toggle Headlights
R         - Change Car Color
Arrows    - Free Camera Movement
ESC       - Exit
```

---

## 7. Requirements Checklist

| Requirement | Status | Implementation |
|-------------|--------|----------------|
| A1: Transformations | ✅ | Translation, rotation, scaling throughout |
| A2: Camera Setup | ✅ | 3 modes: third-person, first-person, free camera |
| A3: Lighting & Materials | ✅ | 8 lights, material system, emissive, fog |
| A4: Object Modeling | ✅ | Car, buildings, road, lamps, barriers from primitives |
| A5: Animation | ✅ | Wheel rotation, traffic, camera transitions |
| A6: Interaction | ✅ | WASD + C/L/R keys + arrows |
| HUD | ✅ | Speed, camera mode, controls, headlight status |
| Night city theme | ✅ | Dark sky, fog, lit windows, lamp glow |
| Documentation | ✅ | This document |
