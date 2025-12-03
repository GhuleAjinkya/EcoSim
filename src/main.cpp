#include "raylib.h"
#include "Simulation.hpp"

int main() {
    const int FPS  = 12;
    const int screenWidth = 940;
    const int screenHeight = 940;
    const int cellSize = 20;
    const int sidebarWidth = 200;
    const int toolbarWidth = 50;
    Simulation simulation(screenHeight, screenWidth, cellSize, sidebarWidth);
    
    InitWindow(screenWidth + sidebarWidth + toolbarWidth, screenHeight, "Ecosystem Simulation");
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);
            simulation.draw();
            simulation.update();
        EndDrawing();
    }
    
    CloseWindow();
}