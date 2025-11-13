#include <raylib.h>
#include <string>
#include "Grid.hpp"

class Simulation {
    Grid grid;
    Grid tmpGrid;
    int selectedRow;
    int selectedColumn;
    int sidebarWidth;
    public:
        Simulation(int height, int width, int cellSize, int sidebarWidth) 
        : grid(height, width, cellSize), tmpGrid(height, width, cellSize),
          selectedRow(-1), selectedColumn(-1) {};
        void draw();
        void update();
};

void Simulation::draw() {
    grid.draw();

    int sidebarX = grid.getColumns() * grid.getCellSize();
    int screenH = GetScreenHeight();
    DrawRectangle(sidebarX, 0, sidebarWidth, screenH, LIGHTGRAY);

    // header
    DrawText("Tile Info", sidebarX + 10, 10, 20, BLACK);

    if (selectedRow >= 0 && selectedColumn >= 0) {
        Tile &t = grid.tiles[selectedRow][selectedColumn];
        // tile coordinates
        char buf[128];
        sprintf(buf, "Pos: %d, %d", selectedRow, selectedColumn);
        DrawText(buf, sidebarX + 10, 40, 18, BLACK);

        // type
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
    } else {
        DrawText("Click a tile\nto view details", sidebarX + 10, 40, 18, DARKGRAY);
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
    grid = tmpGrid;
}
