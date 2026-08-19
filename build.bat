@echo off
REM ═══════════════════════════════════════════
REM  Fast & Furious: 3D Street Racing
REM  Build Script (MinGW)
REM ═══════════════════════════════════════════
REM  Prerequisites: MinGW with OpenGL + GLUT
REM ═══════════════════════════════════════════

echo.
echo === Building Fast & Furious: 3D Street Racing ===
echo.

g++ -o StreetRacing.exe main.cpp -lfreeglut -lopengl32 -lglu32 -lwinmm -lm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo BUILD SUCCESSFUL!
    echo Run: StreetRacing.exe
    echo.
) else (
    echo.
    echo BUILD FAILED!
    echo Make sure MinGW and FreeGLUT are installed.
    echo.
)

pause
