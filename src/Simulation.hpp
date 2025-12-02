#pragma once
#include <raylib.h>
#include <string>
#include "AnimalsManager.hpp"
using namespace std;
class Simulation {
    Grid grid;
    Grid tmpGrid;
    AnimalsManager animals;
    int selectedRow;
    int selectedColumn;
    int sidebarWidth;
    int selectedEntityID = -1;
    public:
        Simulation(int height, int width, int cellSize, int sidebarWidth) 
        : grid(height, width, cellSize), tmpGrid(height, width, cellSize),
          selectedRow(-1), selectedColumn(-1), sidebarWidth(sidebarWidth)
          {
            animals.spawnHerbivoreGroups(grid, 30);
            animals.spawnCarnivoreGroups(grid, 5);
          };
        void draw();
        void update();
};

void Simulation::draw() {
    grid.draw();

    for (Entity& e : animals.getEntities()) {
        int px = e.getPos().x * grid.getCellSize();
        int py = e.getPos().y * grid.getCellSize();
        int offset = grid.getCellSize() / 2;
        DrawCircle(px + offset, py+offset, (grid.getCellSize() / 2) - 3 , e.getColor());
    }

    int sidebarX = grid.getColumns() * grid.getCellSize();
    int screenH = GetScreenHeight();
    DrawRectangle(sidebarX, 0, sidebarWidth, screenH, LIGHTGRAY);

    // sidebar
    DrawText("Tile Info", sidebarX + 10, 10, 20, BLACK);

    if (selectedRow >= 0 && selectedColumn >= 0) {
        Tile &t = grid.tiles[selectedRow][selectedColumn];
        char buf[128];
        sprintf(buf, "Pos: %d, %d", selectedRow, selectedColumn);
        DrawText(buf, sidebarX + 10, 40, 18, BLACK);

        string typeStr = "Unknown";
        TileType tt = t.getType();
        if (tt == TileType::Grass) typeStr = "Grass";
        else if (tt == TileType::Water) typeStr = "Water";
        else if (tt == TileType::Rock) typeStr = "Rock";
        DrawText(("Type: " + typeStr).c_str(), sidebarX + 10, 70, 18, BLACK);

        if (tt == TileType::Water) {
            DrawText("Health: N/A", sidebarX + 10, 100, 18, BLACK);
        } else {
            sprintf(buf, "Health: %d", t.getHealth());
            DrawText(buf, sidebarX + 10, 100, 18, BLACK);
        }
        if (t.getOccupiedID() != -1) {
            selectedEntityID = t.getOccupiedID();
            DrawText(string("Occupied: Yes").c_str(), sidebarX + 10, 130, 18, RED);
        } else {
            DrawText(string("Occupied: No").c_str(), sidebarX + 10, 130, 18, DARKGRAY);   
        }
    } else {
        DrawText("Click a tile\nto view details", sidebarX + 10, 40, 18, DARKGRAY);
    }

    int entitySectionY = screenH / 2;
    DrawText("Entity Info", sidebarX + 10, entitySectionY + 8, 20, BLACK);

    if (selectedEntityID != -1) {
        Entity copy = Entity(-1, 0, 0, 0, Species::Herbivore);
        for (Entity e : animals.getEntities()) { 
            if (e.getID() == selectedEntityID) { 
                copy = e; 
                break; 
            } 
        }

        if (copy.getID() != -1) {
            char buf[128];
            sprintf(buf, "ID: %d", copy.getID());
            DrawText(buf, sidebarX + 10, entitySectionY + 40, 18, BLACK);
            std::string sp = (copy.getSpecies() == Species::Herbivore) ? "Herbivore" : "Carnivore";
            DrawText(("Species: " + sp).c_str(), sidebarX + 10, entitySectionY + 70, 18, BLACK);
            sprintf(buf, "Age: %d", copy.getAge());
            DrawText(buf, sidebarX + 10, entitySectionY + 130, 18, BLACK);
            sprintf(buf, "Health: %d", copy.getHealth());
            DrawText(buf, sidebarX + 10, entitySectionY + 100, 18, BLACK);
            sprintf(buf, "Pos: %d, %d", copy.getPos().x, copy.getPos().y);
            DrawText(buf, sidebarX + 10, entitySectionY + 160, 18, BLACK);
        } else {
            DrawText("Selected entity: (dead)", sidebarX + 10, entitySectionY + 40, 18, DARKGRAY);
        }
    } else {
        DrawText("No entity selected", sidebarX + 10, entitySectionY + 40, 18, DARKGRAY);
    }
}

int getSurrGrass(int i, int j, Grid& grid) {
    int total = 0;
    if (i > 0 && grid.tiles[i-1][j].getType() == TileType::Grass) total++;
    if (i < grid.getRows()-1 && grid.tiles[i+1][j].getType() == TileType::Grass) total++;
    if (j > 0 && grid.tiles[i][j-1].getType() == TileType::Grass) total++;
    if (j < grid.getColumns()-1 && grid.tiles[i][j+1].getType() == TileType::Grass) total++;
    return total;
}

void Simulation::update() {
    int sidebarX = grid.getColumns() * grid.getCellSize();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        int col = (int)(mp.x) / grid.getCellSize();
        int row = (int)(mp.y) / grid.getCellSize();
        if (row >= 0 && row < grid.getRows() && col >= 0 && col < grid.getColumns()) {
            selectedRow = row;
            selectedColumn = col;
        } else {
            selectedRow = selectedColumn = -1; // clicked outside grid
        }
    }

    for (int i = 0; i < tmpGrid.getRows(); i++) {
        for (int j = 0; j < tmpGrid.getColumns(); j++) {
            tmpGrid.tiles[i][j].update(getSurrGrass(i,j,grid));
        }
    }
    animals.update(tmpGrid);
    grid = tmpGrid;
    
}
