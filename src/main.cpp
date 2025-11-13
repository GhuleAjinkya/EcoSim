#include "raylib.h"
#include "resource_dir.h"	
#include "Simulation.hpp"

int main() {
    const int FPS  = 12;
    const int screenWidth = 950;
    const int screenHeight = 950;
    const int cellSize = 20;
    const int sidebarWidth = 200;

    Simulation simulation(screenHeight, screenWidth, cellSize, sidebarWidth);
    
    InitWindow(screenWidth + sidebarWidth, screenHeight, "Ecosystem Simulation");
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