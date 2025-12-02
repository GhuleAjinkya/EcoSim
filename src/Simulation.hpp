#pragma once
#include <raylib.h>
#include <string>
#include "AnimalsManager.hpp"
using namespace std;

enum class Tool {
    None,
    PlaceHerbivore,
    PlaceCarnivore,
    SetGrass,
    SetWater,
    SetRock
};

class Simulation {
    Grid grid;
    Grid tmpGrid;
    AnimalsManager animals;
    int selectedRow;
    int selectedColumn;
    int sidebarWidth;
    int selectedEntityID = -1;
    Tool activeTool = Tool::None;
    public:
        Simulation(int height, int width, int cellSize, int sidebarWidth) 
        : grid(height, width, cellSize), tmpGrid(height, width, cellSize),
          selectedRow(-1), selectedColumn(-1), sidebarWidth(sidebarWidth)
          {
            animals.spawnHerbivoreGroups(grid, Constants::HERBIVORE_MAX_TOTAL);
            animals.spawnCarnivoreGroups(grid, Constants::CARNIVORE_MAX_TOTAL);
          };
        void draw();
        void update();
        void drawSidebar();
        void drawToolbar();
        void applyTool(int col, int row);
};

void Simulation::drawToolbar() {
    int sidebarX = grid.getColumns() * grid.getCellSize();
    int toolbarX = sidebarX + sidebarWidth;  // toolbar is RIGHT of sidebar
    int toolbarWidth = 50;
    int screenH = GetScreenHeight();
    
    DrawRectangle(toolbarX, 0, toolbarWidth, screenH, DARKGRAY);
    
    // Button size and spacing
    int buttonSize = 40;
    int buttonSpacing = 5;
    int startY = 10;
    
    // Button 1: Place Herbivore (yellow circle)
    Rectangle herbButton = {(float)toolbarX + 5, (float)startY, (float)buttonSize, (float)buttonSize};
    DrawRectangle((int)herbButton.x, (int)herbButton.y, (int)herbButton.width, (int)herbButton.height, 
                  activeTool == Tool::PlaceHerbivore ? LIME : DARKGRAY);
    DrawCircle((int)(herbButton.x + buttonSize/2), (int)(herbButton.y + buttonSize/2), 12, {255, 255, 0, 255});
    DrawRectangleLines((int)herbButton.x, (int)herbButton.y, (int)herbButton.width, (int)herbButton.height, WHITE);
    
    // Button 2: Place Carnivore (red circle)
    Rectangle carnButton = {(float)toolbarX + 5, (float)(startY + buttonSize + buttonSpacing), (float)buttonSize, (float)buttonSize};
    DrawRectangle((int)carnButton.x, (int)carnButton.y, (int)carnButton.width, (int)carnButton.height, 
                  activeTool == Tool::PlaceCarnivore ? LIME : DARKGRAY);
    DrawCircle((int)(carnButton.x + buttonSize/2), (int)(carnButton.y + buttonSize/2), 12, {255, 0, 0, 255});
    DrawRectangleLines((int)carnButton.x, (int)carnButton.y, (int)carnButton.width, (int)carnButton.height, WHITE);
    
    // Button 3: Set Grass (green square)
    Rectangle grassButton = {(float)toolbarX + 5, (float)(startY + 2*(buttonSize + buttonSpacing)), (float)buttonSize, (float)buttonSize};
    DrawRectangle((int)grassButton.x, (int)grassButton.y, (int)grassButton.width, (int)grassButton.height, 
                  activeTool == Tool::SetGrass ? LIME : DARKGRAY);
    DrawRectangle((int)(grassButton.x + 5), (int)(grassButton.y + 5), buttonSize - 10, buttonSize - 10, {21, 191, 64, 255});
    DrawRectangleLines((int)grassButton.x, (int)grassButton.y, (int)grassButton.width, (int)grassButton.height, WHITE);
    
    // Button 4: Set Water (blue square)
    Rectangle waterButton = {(float)toolbarX + 5, (float)(startY + 3*(buttonSize + buttonSpacing)), (float)buttonSize, (float)buttonSize};
    DrawRectangle((int)waterButton.x, (int)waterButton.y, (int)waterButton.width, (int)waterButton.height, 
                  activeTool == Tool::SetWater ? LIME : DARKGRAY);
    DrawRectangle((int)(waterButton.x + 5), (int)(waterButton.y + 5), buttonSize - 10, buttonSize - 10, {0, 0, 255, 255});
    DrawRectangleLines((int)waterButton.x, (int)waterButton.y, (int)waterButton.width, (int)waterButton.height, WHITE);
    
    // Button 5: Set Rock (brown square)
    Rectangle rockButton = {(float)toolbarX + 5, (float)(startY + 4*(buttonSize + buttonSpacing)), (float)buttonSize, (float)buttonSize};
    DrawRectangle((int)rockButton.x, (int)rockButton.y, (int)rockButton.width, (int)rockButton.height, 
                  activeTool == Tool::SetRock ? LIME : DARKGRAY);
    DrawRectangle((int)(rockButton.x + 5), (int)(rockButton.y + 5), buttonSize - 10, buttonSize - 10, {76, 50, 33, 255});
    DrawRectangleLines((int)rockButton.x, (int)rockButton.y, (int)rockButton.width, (int)rockButton.height, WHITE);
    
    // Handle button clicks
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        if (CheckCollisionPointRec(mp, herbButton)) {
            activeTool = (activeTool == Tool::PlaceHerbivore) ? Tool::None : Tool::PlaceHerbivore;
        } else if (CheckCollisionPointRec(mp, carnButton)) {
            activeTool = (activeTool == Tool::PlaceCarnivore) ? Tool::None : Tool::PlaceCarnivore;
        } else if (CheckCollisionPointRec(mp, grassButton)) {
            activeTool = (activeTool == Tool::SetGrass) ? Tool::None : Tool::SetGrass;
        } else if (CheckCollisionPointRec(mp, waterButton)) {
            activeTool = (activeTool == Tool::SetWater) ? Tool::None : Tool::SetWater;
        } else if (CheckCollisionPointRec(mp, rockButton)) {
            activeTool = (activeTool == Tool::SetRock) ? Tool::None : Tool::SetRock;
        }
    }
}

void Simulation::applyTool(int col, int row) {
    if (row < 0 || row >= grid.getRows() || col < 0 || col >= grid.getColumns()) return;
    
    Tile &t = tmpGrid.tiles[row][col];
    
    switch (activeTool) {
        case Tool::PlaceHerbivore:
            if (t.getOccupiedID() == -1 && t.getType() != TileType::Water) {
                animals.spawnEntity(Species::Herbivore, col, row, grid);
            }
            break;
        case Tool::PlaceCarnivore:
            if (t.getOccupiedID() == -1 && t.getType() != TileType::Water) {
                animals.spawnEntity(Species::Carnivore, col, row, grid);
            }
            break;
        case Tool::SetGrass:
            if (t.getOccupiedID() != -1) {
                t.freeOccupant();
            }
            t.setType(TileType::Grass);
            break;
        case Tool::SetWater:
            if (t.getOccupiedID() != -1) {
                t.freeOccupant();
            }
            t.setType(TileType::Water);
            break;
        case Tool::SetRock:
            if (t.getOccupiedID() != -1) {
                t.freeOccupant();
            }
            t.setType(TileType::Rock);
            break;
        default:
            break;
    }
}

void Simulation::drawSidebar() {
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

void Simulation::draw() {
    grid.draw();

    for (Entity& e : animals.getEntities()) {
        int px = e.getPos().x * grid.getCellSize();
        int py = e.getPos().y * grid.getCellSize();
        int offset = grid.getCellSize() / 2;
        DrawCircle(px + offset, py+offset, (grid.getCellSize() / 2) - 3 , e.getColor());
    }
    drawSidebar();
    drawToolbar();
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
    int gridEndX = grid.getColumns() * grid.getCellSize();
    int sidebarX = gridEndX;
    int toolbarX = sidebarX + sidebarWidth;
    
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 mp = GetMousePosition();
        
        // only process grid clicks if not clicking toolbar or sidebar
        if (mp.x < gridEndX) {
            int col = (int)(mp.x) / grid.getCellSize();
            int row = (int)(mp.y) / grid.getCellSize();
            
            if (activeTool != Tool::None) {
                // apply tool
                applyTool(col, row);
            } else {
                // select tile
                if (row >= 0 && row < grid.getRows() && col >= 0 && col < grid.getColumns()) {
                    selectedRow = row;
                    selectedColumn = col;
                } else {
                    selectedRow = selectedColumn = -1;
                }
            }
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
