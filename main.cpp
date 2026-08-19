// ═══════════════════════════════════════════════════════════════════
//  FAST & FURIOUS: 3D STREET RACING EXPERIENCE
//  Computer Graphics Final Project
//  Platform: OpenGL (GLUT) + C++
// ═══════════════════════════════════════════════════════════════════
//
//  Features demonstrated:
//    - Transformations (translation, rotation, scaling)
//    - Camera system (third-person, first-person, free camera)
//    - Lighting (directional moonlight, point lamps, headlights)
//    - Materials (road, buildings, glass, car body)
//    - Object modeling from primitives (car, buildings, road, lamps)
//    - Animation (wheel rotation, camera transitions, traffic)
//    - User interaction (WASD, camera switch, headlight toggle, color)
//    - HUD overlay (speed, camera mode, controls)
//
//  Controls:
//    W/S       - Accelerate / Brake
//    A/D       - Steer left / right
//    C         - Switch camera mode
//    L         - Toggle headlights
//    R         - Change car color
//    Arrow keys - Free camera movement (in free mode)
//    ESC       - Exit
// ═══════════════════════════════════════════════════════════════════

#include <GL/glut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
  #include <mmsystem.h>
#endif

// ─────────────────────────────────────────────
//  Constants
// ─────────────────────────────────────────────
const float PI  = 3.14159265358979f;
const float DEG = PI / 180.0f;
const int   WIN_W = 1280;
const int   WIN_H = 720;

// ─────────────────────────────────────────────
//  Window
// ─────────────────────────────────────────────
int winW = WIN_W, winH = WIN_H;

// ─────────────────────────────────────────────
//  Camera Modes
// ─────────────────────────────────────────────
enum CameraMode { CAM_THIRD, CAM_FIRST, CAM_FREE };
const char* cameraNames[] = { "Third Person", "First Person", "Free Camera" };
CameraMode camMode = CAM_THIRD;

// Camera state for smooth transitions
float camX = 0, camY = 0, camZ = 0;
float camLookX = 0, camLookY = 0, camLookZ = 0;
float camFreeX = 0, camFreeY = 5, camFreeZ = 15;
float camFreeYaw = 0, camFreePitch = -15;
float camTransTimer = 0;
float camTransitionDur = 0.5f;
bool  camTransitioning = false;
float camFromX, camFromY, camFromZ;
float camToX, camToY, camToZ;
float camLookFromX, camLookFromY, camLookFromZ;
float camLookToX, camLookToY, camLookToZ;

// ─────────────────────────────────────────────
//  Game State
// ─────────────────────────────────────────────
enum GameState { MENU, DRIVING };
GameState gameState = MENU;

float gameTime = 0;

// ─────────────────────────────────────────────
//  Player Car
// ─────────────────────────────────────────────
float carX = 0, carY = 0, carZ = 0;
float     carHeading = 0.0f;          // radians, 0 = facing +Z (start from far end)
float carSpeed = 0;          // m/s
float carSteerAngle = 0;     // current wheel turn
float wheelRotation = 0;     // visual wheel spin
float maxSpeed = 60.0f;
float acceleration = 35.0f;
float brakeForce = 40.0f;
float friction = 3.0f;
float steerAmount = 0;
bool  headlightsOn = true;
int   carColorIndex = 0;

// Car color palette
float carColors[][3] = {
    {0.85f, 0.05f, 0.05f},  // Red
    {0.05f, 0.15f, 0.85f},  // Blue
    {0.9f,  0.8f,  0.0f},   // Yellow
    {0.05f, 0.85f, 0.15f},  // Green
    {0.9f,  0.45f, 0.0f},   // Orange
    {0.15f, 0.15f, 0.15f},  // Black
    {0.9f,  0.9f,  0.9f},   // White
    {0.5f,  0.0f,  0.7f},   // Purple
};
const int NUM_CAR_COLORS = 8;

// ─────────────────────────────────────────────
//  Road / City Layout
// ─────────────────────────────────────────────
const float ROAD_WIDTH  = 22.0f;
const float ROAD_LENGTH = 500.0f;
const float SIDEWALK_H  = 0.15f;
const float LANE_WIDTH  = 3.5f;

// ─────────────────────────────────────────────
//  Input State
// ─────────────────────────────────────────────
bool keyW = false, keyS = false, keyA = false, keyD = false;
bool keyLeft = false, keyRight = false, keyUp = false, keyDown = false;
bool keyC = false, keyL = false, keyR = false;

// ─────────────────────────────────────────────
//  Collision Effect
// ─────────────────────────────────────────────
float collisionFlash = 0.0f;
float screenShakeAmount = 0.0f;
float collisionCooldown = 0.0f;

// ─────────────────────────────────────────────
//  Music
// ─────────────────────────────────────────────
bool musicPlaying = false;
int currentTrack = 0;
const char* musicFiles[] = {
    "music\\track1.mp3",
    "music\\track2.mp3",
    "music\\track3.mp3",
};
const char* trackNames[] = {
    "Track 1",
    "Track 2",
    "Track 3",
};
const int NUM_TRACKS = 3;

// ─────────────────────────────────────────────
//  Traffic Cars (AI, moving objects)
// ─────────────────────────────────────────────
struct TrafficCar {
    float x, z;
    float speed;
    float lane;     // which lane offset from center
    float colorR, colorG, colorB;
    bool  active;
};

std::vector<TrafficCar> trafficCars;

// ─────────────────────────────────────────────
//  Street Lamps (positions for point lights)
// ─────────────────────────────────────────────
struct LampPos {
    float x, z;
};

std::vector<LampPos> lampPositions;
const int NUM_LAMPS = 20;

// ═════════════════════════════════════════════
//  UTILITY FUNCTIONS
// ═════════════════════════════════════════════

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// ═════════════════════════════════════════════
//  MATERIAL SYSTEM
// ═════════════════════════════════════════════
// Sets ambient + diffuse + specular material for the front face

void setMaterial(float r, float g, float b,
                 float sr = 0.2f, float sg = 0.2f, float sb = 0.2f,
                 float shine = 30.0f)
{
    GLfloat amb[4]  = { r * 0.35f, g * 0.35f, b * 0.35f, 1.0f };
    GLfloat dif[4]  = { r, g, b, 1.0f };
    GLfloat spc[4]  = { sr, sg, sb, 1.0f };
    GLfloat shin[1] = { shine };
    glMaterialfv(GL_FRONT, GL_AMBIENT,  amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  dif);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spc);
    glMaterialfv(GL_FRONT, GL_SHININESS, shin);
}

// Set emissive material (for glowing objects like lights)
void setEmissive(float r, float g, float b) {
    GLfloat e[4] = { r, g, b, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, e);
}

// Clear emissive (set to black)
void clearEmissive() {
    GLfloat e[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    glMaterialfv(GL_FRONT, GL_EMISSION, e);
}

// ═════════════════════════════════════════════
//  DRAWING PRIMITIVES
// ═════════════════════════════════════════════

// Draw a box centered at origin with given dimensions
void drawBox(float w, float h, float d) {
    glPushMatrix();
    glScalef(w, h, d);
    glutSolidCube(1.0);
    glPopMatrix();
}

// Draw a cylinder along +Z axis
void drawCylinder(float radius, float height, int slices = 12) {
    GLUquadric* quad = gluNewQuadric();
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluCylinder(quad, radius, radius, height, slices, 1);
    gluDisk(quad, 0.0, radius, slices, 1);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, height);
    gluDisk(quad, 0.0, radius, slices, 1);
    glPopMatrix();
    gluDeleteQuadric(quad);
}

// Draw a sphere
void drawSphere(float radius, int slices = 12, int stacks = 8) {
    glutSolidSphere(radius, slices, stacks);
}

// ═════════════════════════════════════════════
//  OBJECT: SPORTS CAR
//  Built entirely from primitives (boxes, cylinders, spheres)
// ═════════════════════════════════════════════

void drawWheel(float x, float y, float z, float steer) {
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(steer * 45.0f, 0.0f, 1.0f, 0.0f);
    glRotatef(wheelRotation, 1.0f, 0.0f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);

    glDisable(GL_CULL_FACE);

    setMaterial(0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 5.0f);
    drawCylinder(0.22f, 0.14f, 14);

    setMaterial(0.8f, 0.8f, 0.85f, 0.9f, 0.9f, 0.9f, 80.0f);
    drawCylinder(0.10f, 0.15f, 6);

    setMaterial(0.6f, 0.6f, 0.65f, 0.7f, 0.7f, 0.7f, 50.0f);
    drawCylinder(0.05f, 0.16f, 5);

    glEnable(GL_CULL_FACE);
    glPopMatrix();
}

void drawSportsCar() {
    glPushMatrix();
    glTranslatef(carX, carY, carZ);
    glRotatef(carHeading * 180.0f / PI, 0.0f, 1.0f, 0.0f);

    float cr = carColors[carColorIndex][0];
    float cg = carColors[carColorIndex][1];
    float cb = carColors[carColorIndex][2];

    // ── Underbody ──
    setMaterial(0.05f, 0.05f, 0.05f, 0.02f, 0.02f, 0.02f, 5.0f);
    drawBox(1.2f, 0.08f, 2.6f);

    // ── Main body (lower) ──
    setMaterial(cr, cg, cb, 0.6f, 0.6f, 0.6f, 60.0f);
    drawBox(1.25f, 0.35f, 2.7f);

    // ── Side skirts ──
    setMaterial(cr * 0.85f, cg * 0.85f, cb * 0.85f, 0.3f, 0.3f, 0.3f, 30.0f);
    drawBox(1.3f, 0.12f, 2.4f);

    // ── Hood (sloped front) ──
    glPushMatrix();
    glTranslatef(0.0f, 0.22f, 0.9f);
    glRotatef(-8.0f, 1.0f, 0.0f, 0.0f);
    setMaterial(cr * 0.95f, cg * 0.95f, cb * 0.95f, 0.5f, 0.5f, 0.5f, 70.0f);
    drawBox(1.15f, 0.12f, 0.85f);
    glPopMatrix();

    // ── Cabin ──
    glPushMatrix();
    glTranslatef(0.0f, 0.4f, -0.05f);
    setMaterial(cr * 0.9f, cg * 0.9f, cb * 0.9f, 0.4f, 0.4f, 0.4f, 50.0f);
    drawBox(1.0f, 0.3f, 1.1f);
    glPopMatrix();

    // ── Roof ──
    glPushMatrix();
    glTranslatef(0.0f, 0.57f, -0.1f);
    setMaterial(cr * 0.85f, cg * 0.85f, cb * 0.85f, 0.35f, 0.35f, 0.35f, 40.0f);
    drawBox(0.95f, 0.05f, 0.9f);
    glPopMatrix();

    // ── Windshield (blue glass) ──
    setMaterial(0.3f, 0.45f, 0.85f, 0.8f, 0.8f, 1.0f, 120.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.44f, 0.44f);
    glRotatef(-22.0f, 1.0f, 0.0f, 0.0f);
    drawBox(0.9f, 0.26f, 0.04f);
    glPopMatrix();

    // ── Rear window ──
    setMaterial(0.25f, 0.38f, 0.75f, 0.7f, 0.7f, 0.9f, 100.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.44f, -0.62f);
    glRotatef(22.0f, 1.0f, 0.0f, 0.0f);
    drawBox(0.87f, 0.22f, 0.04f);
    glPopMatrix();

    // ── Side windows ──
    setMaterial(0.28f, 0.42f, 0.8f, 0.7f, 0.7f, 0.9f, 100.0f);
    glPushMatrix();
    glTranslatef(-0.52f, 0.44f, 0.0f);
    drawBox(0.03f, 0.2f, 0.7f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.52f, 0.44f, 0.0f);
    drawBox(0.03f, 0.2f, 0.7f);
    glPopMatrix();

    // ── Headlights (emissive yellow) ──
    setMaterial(1.0f, 1.0f, 0.8f, 1.0f, 1.0f, 1.0f, 100.0f);
    if (headlightsOn) setEmissive(0.9f, 0.9f, 0.5f);
    glPushMatrix();
    glTranslatef(-0.42f, 0.14f, 1.36f);
    drawBox(0.18f, 0.1f, 0.06f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.42f, 0.14f, 1.36f);
    drawBox(0.18f, 0.1f, 0.06f);
    glPopMatrix();
    clearEmissive();

    // ── Tail lights (emissive red) ──
    setMaterial(0.9f, 0.05f, 0.05f, 1.0f, 0.2f, 0.2f, 80.0f);
    setEmissive(0.7f, 0.02f, 0.02f);
    glPushMatrix();
    glTranslatef(-0.45f, 0.14f, -1.36f);
    drawBox(0.14f, 0.08f, 0.04f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.45f, 0.14f, -1.36f);
    drawBox(0.14f, 0.08f, 0.04f);
    glPopMatrix();
    clearEmissive();

    // ── Front bumper ──
    setMaterial(0.1f, 0.1f, 0.1f, 0.15f, 0.15f, 0.15f, 20.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.02f, 1.35f);
    drawBox(1.15f, 0.15f, 0.1f);
    glPopMatrix();

    // ── Rear bumper ──
    glPushMatrix();
    glTranslatef(0.0f, -0.02f, -1.35f);
    drawBox(1.08f, 0.12f, 0.1f);
    glPopMatrix();

    // ── Side mirrors ──
    setMaterial(0.08f, 0.08f, 0.08f, 0.2f, 0.2f, 0.2f, 25.0f);
    glPushMatrix();
    glTranslatef(-0.68f, 0.37f, 0.3f);
    drawBox(0.08f, 0.06f, 0.1f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.68f, 0.37f, 0.3f);
    drawBox(0.08f, 0.06f, 0.1f);
    glPopMatrix();

    // ── Rear spoiler ──
    setMaterial(cr * 0.9f, cg * 0.9f, cb * 0.9f, 0.3f, 0.3f, 0.3f, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.58f, -1.15f);
    drawBox(1.1f, 0.04f, 0.2f);
    glPopMatrix();
    setMaterial(0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 10.0f);
    glPushMatrix();
    glTranslatef(-0.38f, 0.52f, -1.1f);
    drawBox(0.04f, 0.12f, 0.04f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.38f, 0.52f, -1.1f);
    drawBox(0.04f, 0.12f, 0.04f);
    glPopMatrix();

    // ── Neon underglow (optional visual flair) ──
    float neonPulse = 0.4f + sinf(gameTime * 3.0f) * 0.15f;
    setEmissive(cr * neonPulse * 0.5f, cg * neonPulse * 0.5f, cb * neonPulse * 0.5f);
    setMaterial(cr * 0.3f, cg * 0.3f, cb * 0.3f, 0.1f, 0.1f, 0.1f, 5.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.12f, 0.0f);
    drawBox(1.1f, 0.02f, 2.3f);
    glPopMatrix();
    clearEmissive();

    // ── Wheels (4 corners) ──
    // Front wheels respond to steering
    drawWheel(-0.65f, -0.15f,  0.85f, carSteerAngle);
    drawWheel( 0.65f, -0.15f,  0.85f, carSteerAngle);
    // Rear wheels are fixed
    drawWheel(-0.65f, -0.15f, -0.85f, 0.0f);
    drawWheel( 0.65f, -0.15f, -0.85f, 0.0f);

    glPopMatrix();
}

// ═════════════════════════════════════════════
//  OBJECT: ROAD
//  Lane markings, sidewalks, asphalt
// ═════════════════════════════════════════════

void drawRoad() {
    // ── Asphalt surface ──
    setMaterial(0.18f, 0.18f, 0.2f, 0.05f, 0.05f, 0.05f, 5.0f);
    glPushMatrix();
    glTranslatef(0.0f, -0.02f, -ROAD_LENGTH / 2.0f);
    drawBox(ROAD_WIDTH, 0.06f, ROAD_LENGTH);
    glPopMatrix();

    // ── Center double yellow line ──
    setMaterial(0.95f, 0.85f, 0.1f, 0.15f, 0.15f, 0.05f, 5.0f);
    for (float z = 0; z > -ROAD_LENGTH; z -= 4.0f) {
        glPushMatrix();
        glTranslatef(-0.1f, 0.02f, z);
        drawBox(0.06f, 0.01f, 2.0f);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(0.1f, 0.02f, z);
        drawBox(0.06f, 0.01f, 2.0f);
        glPopMatrix();
    }

    // ── Lane dividers (white dashed) ──
    setMaterial(0.9f, 0.9f, 0.9f, 0.1f, 0.1f, 0.1f, 5.0f);
    for (float z = 0; z > -ROAD_LENGTH; z -= 5.0f) {
        // Left lane divider
        glPushMatrix();
        glTranslatef(-LANE_WIDTH, 0.02f, z);
        drawBox(0.12f, 0.01f, 2.0f);
        glPopMatrix();
        // Right lane divider
        glPushMatrix();
        glTranslatef(LANE_WIDTH, 0.02f, z);
        drawBox(0.12f, 0.01f, 2.0f);
        glPopMatrix();
    }

    // ── Road edge lines (solid white) ──
    setMaterial(0.85f, 0.85f, 0.85f, 0.1f, 0.1f, 0.1f, 5.0f);
    glPushMatrix();
    glTranslatef(-ROAD_WIDTH / 2.0f + 0.2f, 0.02f, -ROAD_LENGTH / 2.0f);
    drawBox(0.12f, 0.01f, ROAD_LENGTH);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(ROAD_WIDTH / 2.0f - 0.2f, 0.02f, -ROAD_LENGTH / 2.0f);
    drawBox(0.12f, 0.01f, ROAD_LENGTH);
    glPopMatrix();

    // ── Sidewalks (both sides) ──
    setMaterial(0.55f, 0.55f, 0.58f, 0.1f, 0.1f, 0.1f, 10.0f);
    float sidewalkW = 6.0f;
    // Left sidewalk
    glPushMatrix();
    glTranslatef(-ROAD_WIDTH / 2.0f - sidewalkW / 2.0f, SIDEWALK_H / 2.0f - 0.02f, -ROAD_LENGTH / 2.0f);
    drawBox(sidewalkW, SIDEWALK_H, ROAD_LENGTH);
    glPopMatrix();
    // Right sidewalk
    glPushMatrix();
    glTranslatef(ROAD_WIDTH / 2.0f + sidewalkW / 2.0f, SIDEWALK_H / 2.0f - 0.02f, -ROAD_LENGTH / 2.0f);
    drawBox(sidewalkW, SIDEWALK_H, ROAD_LENGTH);
    glPopMatrix();
}

// ═════════════════════════════════════════════
//  OBJECT: BUILDINGS
//  Procedurally generated skyline along both sides
// ═════════════════════════════════════════════

struct Building {
    float x, z;
    float w, h, d;
    float r, g, b;
    int   floors;
};

std::vector<Building> buildings;

void initBuildings() {
    buildings.clear();
    srand(42);

    // Color palette for buildings
    float palette[][3] = {
        {0.45f, 0.45f, 0.5f},  {0.5f, 0.48f, 0.45f}, {0.4f, 0.42f, 0.48f},
        {0.55f, 0.52f, 0.5f},  {0.38f, 0.4f, 0.45f},  {0.52f, 0.5f, 0.48f},
        {0.42f, 0.38f, 0.4f},  {0.48f, 0.48f, 0.52f},
    };

    for (float z = -5.0f; z > -ROAD_LENGTH + 10.0f; z -= 8.0f) {
        // Left side buildings
        Building b1;
        b1.x = -ROAD_WIDTH / 2.0f - 6.0f - (rand() % 30) / 10.0f;
        b1.z = z + (rand() % 40) / 10.0f;
        b1.w = 3.0f + (rand() % 30) / 10.0f;
        b1.h = 4.0f + (rand() % 80) / 10.0f;
        b1.d = 3.0f + (rand() % 30) / 10.0f;
        b1.floors = 2 + rand() % 5;
        int ci = rand() % 8;
        b1.r = palette[ci][0]; b1.g = palette[ci][1]; b1.b = palette[ci][2];
        buildings.push_back(b1);

        // Right side buildings
        Building b2;
        b2.x = ROAD_WIDTH / 2.0f + 6.0f + (rand() % 30) / 10.0f;
        b2.z = z + (rand() % 40) / 10.0f;
        b2.w = 3.0f + (rand() % 30) / 10.0f;
        b2.h = 4.0f + (rand() % 80) / 10.0f;
        b2.d = 3.0f + (rand() % 30) / 10.0f;
        b2.floors = 2 + rand() % 5;
        ci = rand() % 8;
        b2.r = palette[ci][0]; b2.g = palette[ci][1]; b2.b = palette[ci][2];
        buildings.push_back(b2);
    }
}

void drawBuilding(const Building& b) {
    glPushMatrix();
    glTranslatef(b.x, 0.0f, b.z);

    // Main structure
    setMaterial(b.r, b.g, b.b, 0.15f, 0.15f, 0.15f, 15.0f);
    glPushMatrix();
    glTranslatef(0.0f, b.h / 2.0f, 0.0f);
    drawBox(b.w, b.h, b.d);
    glPopMatrix();

    // Windows (rows x columns on front and back faces)
    setMaterial(0.6f, 0.75f, 0.9f, 0.8f, 0.8f, 1.0f, 100.0f);
    float winH = b.h / (b.floors + 1);
    float winW = b.w / 3.0f;
    for (int f = 0; f < b.floors; f++) {
        for (int c = -1; c <= 1; c++) {
            float wy = winH * (f + 1);
            float wx = c * winW * 0.55f;
            bool lit = (rand() % 3 != 0);
            if (lit) {
                setEmissive(0.2f, 0.25f, 0.3f);
            } else {
                setEmissive(0.0f, 0.0f, 0.0f);
                setMaterial(0.15f, 0.2f, 0.3f, 0.5f, 0.5f, 0.7f, 80.0f);
            }
            // Front face windows
            glPushMatrix();
            glTranslatef(wx, wy, b.d / 2.0f + 0.01f);
            drawBox(0.35f, 0.28f, 0.02f);
            glPopMatrix();
            // Back face windows
            glPushMatrix();
            glTranslatef(wx, wy, -b.d / 2.0f - 0.01f);
            drawBox(0.35f, 0.28f, 0.02f);
            glPopMatrix();
        }
    }
    clearEmissive();

    // Roof
    setMaterial(b.r * 0.7f, b.g * 0.7f, b.b * 0.7f, 0.1f, 0.1f, 0.1f, 10.0f);
    glPushMatrix();
    glTranslatef(0.0f, b.h + 0.05f, 0.0f);
    drawBox(b.w + 0.2f, 0.1f, b.d + 0.2f);
    glPopMatrix();

    glPopMatrix();
}

// ═════════════════════════════════════════════
//  OBJECT: STREET LAMPS
//  With point light sources at each lamp
// ═════════════════════════════════════════════

void initLamps() {
    lampPositions.clear();
    for (float z = -8.0f; z > -ROAD_LENGTH + 10.0f; z -= 15.0f) {
        lampPositions.push_back({-ROAD_WIDTH / 2.0f - 1.0f, z});
        lampPositions.push_back({ ROAD_WIDTH / 2.0f + 1.0f, z});
    }
}

void drawLampPost(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0.0f, z);

    // Pole
    setMaterial(0.4f, 0.4f, 0.45f, 0.4f, 0.4f, 0.45f, 40.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawCylinder(0.06f, 4.5f, 8);
    glPopMatrix();

    // Arm (horizontal extension)
    float armDir = (x < 0) ? 1.0f : -1.0f;
    glPushMatrix();
    glTranslatef(armDir * 0.5f, 4.3f, 0.0f);
    glRotatef(90.0f, 0.0f, 1.0f, 0.0f);
    drawCylinder(0.04f, 1.0f, 6);
    glPopMatrix();

    // Light fixture
    setMaterial(0.9f, 0.85f, 0.5f, 1.0f, 1.0f, 0.8f, 100.0f);
    setEmissive(0.7f, 0.65f, 0.3f);
    glPushMatrix();
    glTranslatef(armDir * 1.3f, 4.2f, 0.0f);
    drawBox(0.3f, 0.1f, 0.3f);
    glPopMatrix();
    clearEmissive();

    glPopMatrix();
}

// ═════════════════════════════════════════════
//  OBJECT: TRAFFIC BARRIERS
// ═════════════════════════════════════════════

void drawBarrier(float x, float z, float length) {
    glPushMatrix();
    glTranslatef(x, 0.2f, z);

    // Base
    setMaterial(0.85f, 0.85f, 0.85f, 0.3f, 0.3f, 0.3f, 20.0f);
    drawBox(0.3f, 0.4f, length);

    // Red/white stripes
    int segments = (int)(length / 0.5f);
    for (int i = 0; i < segments; i++) {
        float sz = -length / 2.0f + i * 0.5f + 0.25f;
        if (i % 2 == 0) setMaterial(0.85f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 10.0f);
        else             setMaterial(0.9f, 0.9f, 0.9f, 0.1f, 0.1f, 0.1f, 10.0f);
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, sz);
        drawBox(0.32f, 0.42f, 0.45f);
        glPopMatrix();
    }

    glPopMatrix();
}

// ═════════════════════════════════════════════
//  TRAFFIC CARS (AI moving objects)
// ═════════════════════════════════════════════

void initTraffic() {
    trafficCars.clear();
    float tColors[][3] = {
        {0.9f, 0.8f, 0.1f}, {0.2f, 0.5f, 0.9f}, {0.1f, 0.7f, 0.3f},
        {0.9f, 0.45f, 0.1f}, {0.6f, 0.1f, 0.7f}, {0.8f, 0.8f, 0.8f},
        {0.15f, 0.15f, 0.15f}, {0.9f, 0.9f, 0.9f},
    };

    // Spawn traffic cars at various positions along the road (oncoming)
    for (int i = 0; i < 30; i++) {
        TrafficCar tc;
        tc.lane = ((i % 3) - 1) * LANE_WIDTH;
        tc.x = tc.lane;
        tc.z = -10.0f - i * 18.0f;
        tc.speed = 5.0f + (rand() % 100) / 20.0f;
        int ci = rand() % 8;
        tc.colorR = tColors[ci][0];
        tc.colorG = tColors[ci][1];
        tc.colorB = tColors[ci][2];
        tc.active = true;
        trafficCars.push_back(tc);
    }
}

void drawTrafficCar(const TrafficCar& tc) {
    if (!tc.active) return;

    glPushMatrix();
    glTranslatef(tc.x, 0.22f, tc.z);
    // Traffic faces +Z (oncoming toward player)

    // Body
    setMaterial(tc.colorR, tc.colorG, tc.colorB, 0.3f, 0.3f, 0.3f, 40.0f);
    drawBox(1.1f, 0.35f, 2.0f);

    // Cabin
    setMaterial(tc.colorR * 0.9f, tc.colorG * 0.9f, tc.colorB * 0.9f, 0.3f, 0.3f, 0.3f, 30.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.33f, -0.1f);
    drawBox(0.9f, 0.28f, 0.9f);
    glPopMatrix();

    // Windows
    setMaterial(0.25f, 0.35f, 0.65f, 0.6f, 0.6f, 0.8f, 100.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.38f, 0.35f);
    glRotatef(-18.0f, 1.0f, 0.0f, 0.0f);
    drawBox(0.82f, 0.2f, 0.03f);
    glPopMatrix();

    // Tail lights (back of car, -Z side)
    setMaterial(0.9f, 0.05f, 0.05f, 0.5f, 0.1f, 0.1f, 60.0f);
    setEmissive(0.5f, 0.02f, 0.02f);
    glPushMatrix();
    glTranslatef(-0.4f, 0.1f, -1.01f);
    drawBox(0.1f, 0.06f, 0.03f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.4f, 0.1f, -1.01f);
    drawBox(0.1f, 0.06f, 0.03f);
    glPopMatrix();

    // Headlights (front of car, +Z side - visible to oncoming player)
    setMaterial(1.0f, 1.0f, 0.8f, 1.0f, 1.0f, 1.0f, 100.0f);
    setEmissive(0.8f, 0.8f, 0.5f);
    glPushMatrix();
    glTranslatef(-0.4f, 0.1f, 1.01f);
    drawBox(0.1f, 0.06f, 0.03f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.4f, 0.1f, 1.01f);
    drawBox(0.1f, 0.06f, 0.03f);
    glPopMatrix();
    clearEmissive();

    // Wheels
    setMaterial(0.1f, 0.1f, 0.1f, 0.05f, 0.05f, 0.05f, 5.0f);
    glPushMatrix(); glTranslatef(-0.55f, -0.13f,  0.6f); glRotatef(90,0,1,0); drawCylinder(0.18f, 0.12f, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(0.55f, -0.13f,  0.6f); glRotatef(90,0,1,0); drawCylinder(0.18f, 0.12f, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.55f, -0.13f, -0.6f); glRotatef(90,0,1,0); drawCylinder(0.18f, 0.12f, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(0.55f, -0.13f, -0.6f); glRotatef(90,0,1,0); drawCylinder(0.18f, 0.12f, 10); glPopMatrix();

    glPopMatrix();
}

// ═════════════════════════════════════════════
//  LIGHTING SETUP
// ═════════════════════════════════════════════
//  GL_LIGHT0: Directional moonlight
//  GL_LIGHT1-GL_LIGHT5: Point lights for street lamps
//  GL_LIGHT6-GL_LIGHT7: Car headlights

void setupLighting() {
    glEnable(GL_LIGHTING);

    // ── Ambient (global night fill) ──
    GLfloat globalAmb[] = { 0.25f, 0.25f, 0.3f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);

    // ── GL_LIGHT0: Directional moonlight ──
    glEnable(GL_LIGHT0);
    GLfloat moonPos[]  = { -10.0f, 20.0f, -5.0f, 0.0f }; // w=0 means directional
    GLfloat moonAmb[]  = { 0.2f, 0.2f, 0.25f, 1.0f };
    GLfloat moonDif[]  = { 0.75f, 0.75f, 0.85f, 1.0f };
    GLfloat moonSpc[]  = { 0.4f, 0.4f, 0.5f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, moonPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT,  moonAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  moonDif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, moonSpc);

    // ── GL_LIGHT1-GL_LIGHT5: Street lamp point lights ──
    for (int i = 0; i < 5 && i < (int)lampPositions.size(); i++) {
        int lightID = GL_LIGHT1 + i;
        glEnable(lightID);
        GLfloat pos[]  = { lampPositions[i].x, 4.2f, lampPositions[i].z, 1.0f };
        GLfloat amb[]  = { 0.15f, 0.12f, 0.07f, 1.0f };
        GLfloat dif[]  = { 1.0f, 0.95f, 0.7f, 1.0f };
        GLfloat spc[]  = { 0.7f, 0.6f, 0.4f, 1.0f };
        glLightfv(lightID, GL_POSITION, pos);
        glLightfv(lightID, GL_AMBIENT,  amb);
        glLightfv(lightID, GL_DIFFUSE,  dif);
        glLightfv(lightID, GL_SPECULAR, spc);
        glLightf(lightID,  GL_CONSTANT_ATTENUATION,  0.8f);
        glLightf(lightID,  GL_LINEAR_ATTENUATION,    0.04f);
        glLightf(lightID,  GL_QUADRATIC_ATTENUATION, 0.008f);
    }

    // ── GL_LIGHT6, GL_LIGHT7: Car headlights ──
    glEnable(GL_LIGHT6);
    glEnable(GL_LIGHT7);
    GLfloat hlAmb[]  = { 0.1f, 0.1f, 0.06f, 1.0f };
    GLfloat hlDif[]  = { 1.0f, 0.98f, 0.8f, 1.0f };
    GLfloat hlSpc[]  = { 1.0f, 1.0f, 0.8f, 1.0f };
    glLightfv(GL_LIGHT6, GL_AMBIENT,  hlAmb);
    glLightfv(GL_LIGHT6, GL_DIFFUSE,  hlDif);
    glLightfv(GL_LIGHT6, GL_SPECULAR, hlSpc);
    glLightf(GL_LIGHT6,  GL_CONSTANT_ATTENUATION,  0.7f);
    glLightf(GL_LIGHT6,  GL_LINEAR_ATTENUATION,    0.08f);
    glLightf(GL_LIGHT6,  GL_QUADRATIC_ATTENUATION, 0.015f);

    glLightfv(GL_LIGHT7, GL_AMBIENT,  hlAmb);
    glLightfv(GL_LIGHT7, GL_DIFFUSE,  hlDif);
    glLightfv(GL_LIGHT7, GL_SPECULAR, hlSpc);
    glLightf(GL_LIGHT7,  GL_CONSTANT_ATTENUATION,  0.7f);
    glLightf(GL_LIGHT7,  GL_LINEAR_ATTENUATION,    0.08f);
    glLightf(GL_LIGHT7,  GL_QUADRATIC_ATTENUATION, 0.015f);

    glEnable(GL_NORMALIZE);

    // ── Fog for night atmosphere ──
    glEnable(GL_FOG);
    GLfloat fogColor[] = { 0.06f, 0.06f, 0.1f, 1.0f };
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogf(GL_FOG_START, 100.0f);
    glFogf(GL_FOG_END, 300.0f);
}

// ═════════════════════════════════════════════
//  UPDATE HEADLIGHT POSITIONS
//  Called every frame to follow the car
// ═════════════════════════════════════════════

void updateHeadlights() {
    if (headlightsOn) {
        float cosH = cosf(carHeading);
        float sinH = sinf(carHeading);

        // Left headlight
        GLfloat leftPos[] = {
            carX - sinH * 0.42f,
            carY + 0.14f,
            carZ + cosH * 1.36f,
            1.0f
        };
        GLfloat leftDir[] = { -sinH, -0.1f, cosH, 0.0f };
        glLightfv(GL_LIGHT6, GL_POSITION, leftPos);
        glLightfv(GL_LIGHT6, GL_SPOT_DIRECTION, leftDir);

        // Right headlight
        GLfloat rightPos[] = {
            carX + sinH * 0.42f,
            carY + 0.14f,
            carZ + cosH * 1.36f,
            1.0f
        };
        GLfloat rightDir[] = { -sinH, -0.1f, cosH, 0.0f };
        glLightfv(GL_LIGHT7, GL_POSITION, rightPos);
        glLightfv(GL_LIGHT7, GL_SPOT_DIRECTION, rightDir);

        glLightf(GL_LIGHT6, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT7, GL_SPOT_CUTOFF, 30.0f);
        glLightf(GL_LIGHT6, GL_SPOT_EXPONENT, 8.0f);
        glLightf(GL_LIGHT7, GL_SPOT_EXPONENT, 8.0f);
    } else {
        // Disable headlights by setting zero intensity
        GLfloat zero[] = { 0, 0, 0, 1 };
        glLightfv(GL_LIGHT6, GL_DIFFUSE, zero);
        glLightfv(GL_LIGHT7, GL_DIFFUSE, zero);
        glLightfv(GL_LIGHT6, GL_SPECULAR, zero);
        glLightfv(GL_LIGHT7, GL_SPECULAR, zero);
    }
}

// ═════════════════════════════════════════════
//  GAME LOGIC
// ═════════════════════════════════════════════

void resetGame() {
    carX = 0.0f;
    carY = 0.18f;
    carZ = -ROAD_LENGTH / 2.0f;
    carHeading = 0.0f;
    carSpeed = 0.0f;
    carSteerAngle = 0.0f;
    wheelRotation = 0.0f;
    carColorIndex = 0;
    headlightsOn = true;
    gameTime = 0;

    // Snap camera directly behind car (no transition)
    camMode = CAM_THIRD;
    camTransitioning = false;
    float cosH = cosf(carHeading);
    float sinH = sinf(carHeading);
    float dist = 7.0f;
    camX = carX + sinH * dist;
    camY = 3.5f;
    camZ = carZ - cosH * dist;
    camLookX = carX + sinH * 3.0f;
    camLookY = 0.5f;
    camLookZ = carZ + cosH * 3.0f;

    camFreeX = 0; camFreeY = 5; camFreeZ = carZ + 12;
    camFreeYaw = 0; camFreePitch = -15;

    initTraffic();
    gameState = DRIVING;
}

void updateMusic(float dt);
void playMusic();
void stopMusic();
void nextTrack();

void updateGame(float dt) {
    updateMusic(dt);
    if (gameState != DRIVING) return;

    gameTime += dt;

    // ── Acceleration / Braking ──
    if (keyW) {
        carSpeed += acceleration * dt;
    } else if (keyS) {
        carSpeed -= brakeForce * dt;
    } else {
        // Natural friction
        if (carSpeed > 0) {
            carSpeed -= friction * dt;
            if (carSpeed < 0) carSpeed = 0;
        } else if (carSpeed < 0) {
            carSpeed += friction * dt;
            if (carSpeed > 0) carSpeed = 0;
        }
    }
    carSpeed = clampf(carSpeed, -maxSpeed * 0.3f, maxSpeed);

    // ── Steering ──
    float steerTarget = 0;
    if (keyA) steerTarget = -1.0f;
    if (keyD) steerTarget =  1.0f;

    carSteerAngle = lerp(carSteerAngle, steerTarget, dt * 10.0f);

    // Only steer when moving
    if (fabsf(carSpeed) > 0.3f) {
        float steerRate = carSteerAngle * 3.0f * dt;
        // Reduce turn at high speed
        steerRate *= (1.0f - fabsf(carSpeed) / maxSpeed * 0.3f);
        carHeading -= steerRate * (carSpeed > 0 ? 1.0f : -1.0f);
    }

    // ── Move car ──
    float cosH = cosf(carHeading);
    float sinH = sinf(carHeading);
    carX += sinH * carSpeed * dt;
    carZ += cosH * carSpeed * dt;

    // ── Clamp X to road ──
    float halfRoad = ROAD_WIDTH / 2.0f - 1.0f;
    if (carX < -halfRoad) { carX = -halfRoad; carSpeed *= 0.8f; }
    if (carX >  halfRoad) { carX =  halfRoad; carSpeed *= 0.8f; }

    // ── Wrap Z (infinite loop) ──
    if (carZ > 10.0f)              { carZ -= ROAD_LENGTH; }
    if (carZ < -ROAD_LENGTH - 10.0f) { carZ += ROAD_LENGTH; }

    // ── Wheel rotation ──
    wheelRotation += carSpeed * 60.0f * dt;

    // ── Update traffic ──
    for (TrafficCar& tc : trafficCars) {
        tc.z -= tc.speed * dt;
        // Wrap around (infinite loop)
        if (tc.z < -ROAD_LENGTH - 10.0f) {
            tc.z += ROAD_LENGTH + 20.0f;
        }
        if (tc.z > 10.0f) {
            tc.z -= ROAD_LENGTH + 20.0f;
        }
    }

    // ── Collision detection ──
    if (collisionCooldown > 0) collisionCooldown -= dt;
    if (collisionFlash > 0) collisionFlash -= dt * 3.0f;
    if (screenShakeAmount > 0) screenShakeAmount -= dt * 8.0f;

    float playerHW = 0.65f;   // player half width
    float playerHL = 1.35f;   // player half length
    float trafficHW = 0.55f;
    float trafficHL = 1.0f;

    for (TrafficCar& tc : trafficCars) {
        if (!tc.active) continue;
        float dx = fabsf(carX - tc.x);
        float rawDz = carZ - tc.z;
        // Wrap Z distance for infinite loop
        if (rawDz >  ROAD_LENGTH / 2.0f) rawDz -= ROAD_LENGTH;
        if (rawDz < -ROAD_LENGTH / 2.0f) rawDz += ROAD_LENGTH;
        float dz = fabsf(rawDz);
        float overlapX = playerHW + trafficHW - dx;
        float overlapZ = playerHL + trafficHL - dz;

        if (overlapX > 0 && overlapZ > 0 && collisionCooldown <= 0) {
            // Hit!
            collisionFlash = 1.0f;
            screenShakeAmount = 0.5f;
            collisionCooldown = 0.5f;

            float impactForce = fabsf(carSpeed) + fabsf(tc.speed);
            carSpeed *= -0.3f;

            float pushDirZ = (carZ > tc.z) ? 1.0f : -1.0f;
            carZ += pushDirZ * overlapZ * 0.8f;

            float pushDirX = (carX > tc.x) ? 1.0f : -1.0f;
            if (overlapX > 0.1f) carX += pushDirX * overlapX * 0.5f;
        }
    }

    // ── Traffic-to-traffic collision ──
    for (int i = 0; i < (int)trafficCars.size(); i++) {
        for (int j = i + 1; j < (int)trafficCars.size(); j++) {
            TrafficCar& a = trafficCars[i];
            TrafficCar& b = trafficCars[j];
            float dx = fabsf(a.x - b.x);
            float rawDz = a.z - b.z;
            if (rawDz >  ROAD_LENGTH / 2.0f) rawDz -= ROAD_LENGTH;
            if (rawDz < -ROAD_LENGTH / 2.0f) rawDz += ROAD_LENGTH;
            float dz = fabsf(rawDz);
            if (dx < trafficHW * 2.0f && dz < trafficHL * 2.0f) {
                float pushX = (a.x > b.x) ? 1.0f : -1.0f;
                a.x += pushX * 0.1f;
                b.x -= pushX * 0.1f;
                if (a.z > b.z) { a.z += 0.05f; b.z -= 0.05f; }
                else           { a.z -= 0.05f; b.z += 0.05f; }
            }
        }
    }
}

// ═════════════════════════════════════════════
//  CAMERA SYSTEM
//  Smooth transitions between modes
// ═════════════════════════════════════════════

void startCamTransition(float toX, float toY, float toZ,
                         float toLookX, float toLookY, float toLookZ) {
    camTransitioning = true;
    camTransTimer = 0;
    camFromX = camX; camFromY = camY; camFromZ = camZ;
    camToX = toX; camToY = toY; camToZ = toZ;
    camLookFromX = camLookX; camLookFromY = camLookY; camLookFromZ = camLookZ;
    camLookToX = toLookX; camLookToY = toLookY; camLookToZ = toLookZ;
}

void switchCamera() {
    CameraMode oldMode = camMode;
    camMode = (CameraMode)((camMode + 1) % 3);

    float cosH = cosf(carHeading);
    float sinH = sinf(carHeading);

    switch (camMode) {
        case CAM_THIRD: {
            float dist = 7.0f;
            float h = 3.5f;
            startCamTransition(
                carX + sinH * dist, h, carZ - cosH * dist,
                carX + sinH * 3.0f, 0.5f, carZ + cosH * 3.0f
            );
            break;
        }
        case CAM_FIRST:
            startCamTransition(
                carX, 0.8f, carZ,
                carX + sinH * 20.0f, 0.8f, carZ + cosH * 20.0f
            );
            break;
        case CAM_FREE:
            startCamTransition(
                carX, 5.0f, carZ + 12.0f,
                carX, 0.0f, carZ
            );
            break;
    }
}

void updateCamera(float dt) {
    float cosH = cosf(carHeading);
    float sinH = sinf(carHeading);

    if (camTransitioning) {
        camTransTimer += dt;
        float t = camTransTimer / camTransitionDur;
        if (t >= 1.0f) {
            t = 1.0f;
            camTransitioning = false;
        }
        // Smooth step
        t = t * t * (3.0f - 2.0f * t);
        camX = lerp(camFromX, camToX, t);
        camY = lerp(camFromY, camToY, t);
        camZ = lerp(camFromZ, camToZ, t);
        camLookX = lerp(camLookFromX, camLookToX, t);
        camLookY = lerp(camLookFromY, camLookToY, t);
        camLookZ = lerp(camLookFromZ, camLookToZ, t);
        return;
    }

    switch (camMode) {
        case CAM_THIRD: {
            float dist = 7.0f;
            float h = 3.5f;
            float tx = carX + sinH * dist;
            float ty = h;
            float tz = carZ - cosH * dist;
            camX = lerp(camX, tx, dt * 5.0f);
            camY = lerp(camY, ty, dt * 5.0f);
            camZ = lerp(camZ, tz, dt * 5.0f);
            float lkx = carX + sinH * 3.0f;
            float lky = 0.5f;
            float lkz = carZ + cosH * 3.0f;
            camLookX = lerp(camLookX, lkx, dt * 6.0f);
            camLookY = lerp(camLookY, lky, dt * 6.0f);
            camLookZ = lerp(camLookZ, lkz, dt * 6.0f);
            break;
        }
        case CAM_FIRST: {
            float fx = carX;
            float fy = 0.8f;
            float fz = carZ;
            float flx = carX + sinH * 20.0f;
            float fly = 0.8f;
            float flz = carZ + cosH * 20.0f;
            camX = lerp(camX, fx, dt * 10.0f);
            camY = lerp(camY, fy, dt * 10.0f);
            camZ = lerp(camZ, fz, dt * 10.0f);
            camLookX = lerp(camLookX, flx, dt * 10.0f);
            camLookY = lerp(camLookY, fly, dt * 10.0f);
            camLookZ = lerp(camLookZ, flz, dt * 10.0f);
            break;
        }
        case CAM_FREE: {
            float moveSpeed = 20.0f;
            float lookSpeed = 2.0f;
            if (keyUp)    camFreeZ -= moveSpeed * dt;
            if (keyDown)  camFreeZ += moveSpeed * dt;
            if (keyLeft)  camFreeX -= moveSpeed * dt;
            if (keyRight) camFreeX += moveSpeed * dt;
            camX = lerp(camX, camFreeX, dt * 8.0f);
            camY = lerp(camY, camFreeY, dt * 8.0f);
            camZ = lerp(camZ, camFreeZ, dt * 8.0f);
            camLookX = lerp(camLookX, carX, dt * 3.0f);
            camLookY = lerp(camLookY, 0.5f, dt * 3.0f);
            camLookZ = lerp(camLookZ, carZ, dt * 3.0f);
            break;
        }
    }
}

// ═════════════════════════════════════════════
//  HUD (2D Overlay)
//  Speed, camera mode, controls, headlight status
// ═════════════════════════════════════════════

void drawText(float x, float y, const char* text, void* font = GLUT_BITMAP_HELVETICA_18) {
    glRasterPos2f(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, (unsigned char)*c);
    }
}

void drawTextCentered(float x, float y, const char* text, void* font = GLUT_BITMAP_TIMES_ROMAN_24) {
    int len = 0;
    for (const char* c = text; *c != '\0'; c++) len++;
    glRasterPos2f(x - len * 4.5f, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(font, (unsigned char)*c);
    }
}

void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_FOG);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    char buf[128];

    if (gameState == MENU) {
        // Dark overlay
        glColor4f(0.0f, 0.0f, 0.0f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(winW, 0);
        glVertex2f(winW, winH); glVertex2f(0, winH);
        glEnd();

        float t = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

        // Title
        float pulse = 0.7f + sinf(t * 2.0f) * 0.3f;
        glColor3f(1.0f, 0.3f * pulse, 0.0f);
        drawTextCentered(winW / 2.0f, winH * 0.78f,
            "FAST & FURIOUS", GLUT_BITMAP_TIMES_ROMAN_24);

        glColor3f(1.0f, 0.7f, 0.0f);
        drawTextCentered(winW / 2.0f, winH * 0.70f,
            "3D STREET RACING EXPERIENCE", GLUT_BITMAP_HELVETICA_18);

        // Controls
        glColor3f(0.85f, 0.85f, 0.85f);
        drawTextCentered(winW / 2.0f, winH * 0.58f, "CONTROLS:", GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.65f, 0.65f, 0.65f);
        drawTextCentered(winW / 2.0f, winH * 0.52f, "W/S     - Accelerate / Brake", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.48f, "A/D     - Steer Left / Right", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.44f, "C       - Switch Camera Mode", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.40f, "L       - Toggle Headlights", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.36f, "R       - Change Car Color", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.32f, "M       - Toggle Music", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.29f, "N       - Next Track", GLUT_BITMAP_HELVETICA_12);
        drawTextCentered(winW / 2.0f, winH * 0.25f, "ESC     - Exit", GLUT_BITMAP_HELVETICA_12);

        // Start prompt
        float startPulse = (sinf(t * 3.0f) + 1.0f) / 2.0f;
        glColor3f(1.0f, startPulse * 0.3f + 0.7f, 0.0f);
        drawTextCentered(winW / 2.0f, winH * 0.18f,
            "Press ENTER to Drive", GLUT_BITMAP_HELVETICA_18);
    }

    if (gameState == DRIVING) {
        // ── Speed (top left) ──
        float speedKmh = fabsf(carSpeed) * 3.6f;
        glColor3f(1.0f, 1.0f, 1.0f);
        sprintf(buf, "SPEED: %.0f km/h", speedKmh);
        drawText(25, winH - 30, buf, GLUT_BITMAP_TIMES_ROMAN_24);

        // Speed bar
        float spdPct = speedKmh / (maxSpeed * 3.6f);
        glColor4f(0.15f, 0.15f, 0.15f, 0.7f);
        glBegin(GL_QUADS);
        glVertex2f(25, winH - 50); glVertex2f(225, winH - 50);
        glVertex2f(225, winH - 43); glVertex2f(25, winH - 43);
        glEnd();
        float sr = spdPct > 0.7f ? 1.0f : 0.1f;
        float sg = spdPct > 0.7f ? 0.3f : 0.7f;
        glColor4f(sr, sg, 0.1f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(25, winH - 50);
        glVertex2f(25 + 200.0f * spdPct, winH - 50);
        glVertex2f(25 + 200.0f * spdPct, winH - 43);
        glVertex2f(25, winH - 43);
        glEnd();

        // ── Camera mode (top right) ──
        glColor3f(0.8f, 0.9f, 1.0f);
        sprintf(buf, "Camera: %s", cameraNames[camMode]);
        drawText(winW - 250, winH - 30, buf, GLUT_BITMAP_HELVETICA_18);
        glColor3f(0.5f, 0.5f, 0.5f);
        drawText(winW - 250, winH - 50, "[C] Switch", GLUT_BITMAP_HELVETICA_12);

        // ── Headlight status (right side) ──
        if (headlightsOn) {
            glColor3f(1.0f, 0.9f, 0.3f);
            drawText(winW - 250, winH - 75, "Headlights: ON", GLUT_BITMAP_HELVETICA_12);
        } else {
            glColor3f(0.4f, 0.4f, 0.4f);
            drawText(winW - 250, winH - 75, "Headlights: OFF", GLUT_BITMAP_HELVETICA_12);
        }

        // ── Car color indicator (right side) ──
        glColor3f(carColors[carColorIndex][0], carColors[carColorIndex][1], carColors[carColorIndex][2]);
        sprintf(buf, "Car Color [R]");
        drawText(winW - 250, winH - 95, buf, GLUT_BITMAP_HELVETICA_12);

        // ── Controls reminder (bottom left) ──
        glColor4f(0.5f, 0.5f, 0.5f, 0.6f);
        drawText(25, 25, "WASD: Drive  C: Camera  L: Lights  R: Color  M: Music  ESC: Exit",
            GLUT_BITMAP_HELVETICA_12);

        // ── Music status (top center) ──
        if (musicPlaying) {
            glColor3f(0.2f, 1.0f, 0.4f);
            sprintf(buf, ">> %s  [M] Off  [N] Next", trackNames[currentTrack]);
        } else {
            glColor3f(0.5f, 0.5f, 0.5f);
            sprintf(buf, "Music Off  [M] On  [N] Next Track");
        }
        drawText(winW / 2.0f - 150, winH - 30, buf, GLUT_BITMAP_HELVETICA_12);

        // ── Collision warning ──
        if (collisionFlash > 0.3f) {
            glColor4f(1.0f, 0.2f, 0.0f, collisionFlash);
            drawTextCentered(winW / 2.0f, winH * 0.6f, "CRASH!", GLUT_BITMAP_TIMES_ROMAN_24);
        }

        // ── Direction indicator (bottom center) ──
        if (carSpeed < -0.5f) {
            glColor3f(1.0f, 0.3f, 0.0f);
            drawTextCentered(winW / 2.0f, 50, "REVERSE", GLUT_BITMAP_HELVETICA_12);
        }
    }

    glDisable(GL_BLEND);
    glEnable(GL_FOG);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ═════════════════════════════════════════════
//  DISPLAY
// ═════════════════════════════════════════════

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Screen shake
    float shakeX = 0, shakeY = 0;
    if (screenShakeAmount > 0.01f) {
        shakeX = (float)(rand() % 100 - 50) / 50.0f * screenShakeAmount * 0.3f;
        shakeY = (float)(rand() % 100 - 50) / 50.0f * screenShakeAmount * 0.2f;
    }

    // Camera view (with screen shake)
    gluLookAt(camX + shakeX, camY + shakeY, camZ,
              camLookX + shakeX, camLookY + shakeY, camLookZ,
              0.0f, 1.0f, 0.0f);

    // Update dynamic lights
    updateHeadlights();

    // Re-position lamp point lights (they're static but update after camera moves)
    for (int i = 0; i < 5 && i < (int)lampPositions.size(); i++) {
        int lightID = GL_LIGHT1 + i;
        GLfloat pos[] = { lampPositions[i].x, 4.2f, lampPositions[i].z, 1.0f };
        glLightfv(lightID, GL_POSITION, pos);
    }

    // ── Draw Scene (3 copies for infinite loop) ──
    for (int loop = -1; loop <= 1; loop++) {
        float loopOff = loop * ROAD_LENGTH;
        glPushMatrix();
        glTranslatef(0.0f, 0.0f, loopOff);

        drawRoad();

        // Sidewalk edges
        setMaterial(0.5f, 0.5f, 0.53f, 0.1f, 0.1f, 0.1f, 10.0f);
        float swH = 0.18f;
        glPushMatrix();
        glTranslatef(-ROAD_WIDTH / 2.0f - 0.15f, swH / 2, -ROAD_LENGTH / 2.0f);
        drawBox(0.3f, swH, ROAD_LENGTH);
        glPopMatrix();
        glPushMatrix();
        glTranslatef(ROAD_WIDTH / 2.0f + 0.15f, swH / 2, -ROAD_LENGTH / 2.0f);
        drawBox(0.3f, swH, ROAD_LENGTH);
        glPopMatrix();

        // Buildings
        for (const Building& b : buildings) {
            drawBuilding(b);
        }

        // Street lamps
        for (const LampPos& lp : lampPositions) {
            drawLampPost(lp.x, lp.z);
        }

        glPopMatrix();
    }

    // Traffic cars (also looped)
    for (const TrafficCar& tc : trafficCars) {
        for (int loop = -1; loop <= 1; loop++) {
            glPushMatrix();
            glTranslatef(0.0f, 0.0f, loop * ROAD_LENGTH);
            drawTrafficCar(tc);
            glPopMatrix();
        }
    }

    // Player car
    if (gameState == DRIVING) {
        drawSportsCar();
    }

    // HUD
    drawHUD();

    // Collision flash overlay
    if (collisionFlash > 0.01f) {
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, winW, 0, winH);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0f, 0.1f, 0.0f, collisionFlash * 0.4f);
        glBegin(GL_QUADS);
        glVertex2f(0, 0); glVertex2f(winW, 0);
        glVertex2f(winW, winH); glVertex2f(0, winH);
        glEnd();
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    glutSwapBuffers();
}

// ═════════════════════════════════════════════
//  MUSIC SYSTEM
// ═════════════════════════════════════════════

static int musicSwitchState = 0;
static float musicSwitchTimer = 0;
static int pendingTrack = 0;
static int currentAlias = 0;

void updateMusic(float dt) {
    if (musicSwitchState == 0) return;
    musicSwitchTimer -= dt;
    if (musicSwitchTimer > 0) return;

    if (musicSwitchState == 1) {
        char buf[64];
        // Stop AND close the previous device. Closing is important:
        // Windows has a limited number of MCI devices, and the old code
        // only "stop"ped, so every track switch leaked a device until
        // even track 1 refused to play.
        sprintf(buf, "close m%d", currentAlias);
        mciSendString(buf, NULL, 0, NULL);
        musicSwitchState = 2;
        musicSwitchTimer = 0.05f;
    } else if (musicSwitchState == 2) {
        currentAlias++;
        char alias[16];
        sprintf(alias, "m%d", currentAlias);
        char cmd[512];
        sprintf(cmd, "open \"%s\" type mpegvideo alias %s", musicFiles[pendingTrack], alias);
        DWORD err = mciSendString(cmd, NULL, 0, NULL);
        if (err == 0) {
            char setcmd[128];
            sprintf(setcmd, "set %s time format milliseconds", alias);
            mciSendString(setcmd, NULL, 0, NULL);
            char playcmd[128];
            sprintf(playcmd, "play %s repeat", alias);
            mciSendString(playcmd, NULL, 0, NULL);
            musicPlaying = true;
        } else {
            // Open failed: release the failed alias so it cannot block
            // a later attempt with the same name.
            char closeBuf[64];
            sprintf(closeBuf, "close %s", alias);
            mciSendString(closeBuf, NULL, 0, NULL);
            musicPlaying = false;
        }
        musicSwitchState = 0;
    }
}

void playMusic() {
    if (musicSwitchState != 0) return;
    pendingTrack = currentTrack;
    musicSwitchState = 1;
    musicSwitchTimer = 0.05f;
}

void stopMusic() {
    char buf[64];
    // Close (not just stop) so the MCI device is freed for the next track.
    sprintf(buf, "close m%d", currentAlias);
    mciSendString(buf, NULL, 0, NULL);
    musicPlaying = false;
    musicSwitchState = 0;
}

void nextTrack() {
    currentTrack = (currentTrack + 1) % NUM_TRACKS;
    playMusic();
}

// ═════════════════════════════════════════════
//  INPUT HANDLERS
// ═════════════════════════════════════════════

void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': keyW = true; break;
        case 's': case 'S': keyS = true; break;
        case 'a': case 'A': keyA = true; break;
        case 'd': case 'D': keyD = true; break;

        case 'c': case 'C':
            if (gameState == DRIVING) switchCamera();
            break;

        case 'l': case 'L':
            if (gameState == DRIVING) headlightsOn = !headlightsOn;
            break;

        case 'r': case 'R':
            if (gameState == DRIVING) {
                carColorIndex = (carColorIndex + 1) % NUM_CAR_COLORS;
            }
            break;

        case 'm': case 'M':
            if (musicPlaying) stopMusic();
            else playMusic();
            break;

        case 'n': case 'N':
            nextTrack();
            break;

        case 13: // Enter
            if (gameState == MENU) resetGame();
            break;

        case 27: // Escape
            exit(0);
            break;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W': keyW = false; break;
        case 's': case 'S': keyS = false; break;
        case 'a': case 'A': keyA = false; break;
        case 'd': case 'D': keyD = false; break;
    }
}

void specialDown(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  keyLeft = true;
    if (key == GLUT_KEY_RIGHT) keyRight = true;
    if (key == GLUT_KEY_UP)    keyUp = true;
    if (key == GLUT_KEY_DOWN)  keyDown = true;
}

void specialUp(int key, int x, int y) {
    if (key == GLUT_KEY_LEFT)  keyLeft = false;
    if (key == GLUT_KEY_RIGHT) keyRight = false;
    if (key == GLUT_KEY_UP)    keyUp = false;
    if (key == GLUT_KEY_DOWN)  keyDown = false;
}

// ═════════════════════════════════════════════
//  RESIZE
// ═════════════════════════════════════════════

void reshape(int w, int h) {
    if (h == 0) h = 1;
    winW = w;
    winH = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0, (double)w / h, 0.1, 600.0);
    glMatrixMode(GL_MODELVIEW);
}

// ═════════════════════════════════════════════
//  TIMER
// ═════════════════════════════════════════════

void timer(int value) {
    float dt = 0.016f;
    updateGame(dt);
    updateCamera(dt);
    glutTimerFunc(16, timer, 0);
    glutPostRedisplay();
}

// ═════════════════════════════════════════════
//  INITIALIZATION
// ═════════════════════════════════════════════

void init() {
    glClearColor(0.04f, 0.04f, 0.08f, 1.0f); // Dark night sky
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    setupLighting();
    initBuildings();
    initLamps();
    initTraffic();

    // Initial camera position (looking at menu scene)
    camX = 0; camY = 5; camZ = -ROAD_LENGTH / 2.0f + 15;
    camLookX = 0; camLookY = 0; camLookZ = -ROAD_LENGTH / 2.0f;
}

// ═════════════════════════════════════════════
//  MAIN
// ═════════════════════════════════════════════

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_ALPHA);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Fast & Furious: 3D Street Racing Experience");

    init();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutSpecialFunc(specialDown);
    glutSpecialUpFunc(specialUp);
    glutTimerFunc(16, timer, 0);

    glutMainLoop();
    return 0;
}
