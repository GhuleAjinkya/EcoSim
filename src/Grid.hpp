#include <vector>
#include <cstdlib>
#include <math.h>
#include "Tile.hpp"
using namespace std;

class Grid {
        int rows;
        int columns;
        int cellSize;
    public:
        vector<vector<Tile>> tiles;
        Grid(int, int, int);
        void initialize(int seed); 
        void draw();
        int getRows() {return rows;}
        int getColumns() {return columns;}
        int getCellSize() {return cellSize;}
};

void Grid::initialize(int seed) {
    srand(seed);
    
    // First pass: Create base terrain
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < columns; col++) {
            if (rand() % 20 == 0) tiles[row][col].setType(TileType::Grass);
            else tiles[row][col].setType(TileType::Rock);
        }
    }
    
    // lake
    int centerX = rand() % rows;
    int centerY = rand() % columns;
    int lakeSize = rand() % 6 + 6; // 4-9 radius

    for (int dx = -lakeSize; dx <= lakeSize; dx++) {
        for (int dy = -lakeSize; dy <= lakeSize; dy++) {
            int newX = centerX + dx;
            int newY = centerY + dy;

            if (newX >= 0 && newX < rows && newY >= 0 && newY < columns) {
                float distance = sqrt(dx*dx + dy*dy);
                if (distance <= lakeSize + (rand() % 3 - 1)) {
                    tiles[newX][newY].setType(TileType::Water);
                }
            }
        }
    }
    
    // Add grass in an 8-block radius around water
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < columns; col++) {
            if (tiles[row][col].getType() == TileType::Water) {
                for (int dx = -8; dx <= 8; dx++) {
                    for (int dy = -8; dy <= 8; dy++) {
                        int nx = row + dx;
                        int ny = col + dy;
                        if (nx >= 0 && nx < rows && ny >= 0 && ny < columns) {
                            float dist = sqrt(dx*dx + dy*dy);
                            if (dist <= 5 && tiles[nx][ny].getType() == TileType::Rock) {
                                tiles[nx][ny].setType(TileType::Grass);
                            } else if (dist > 5 && dist < 8 && tiles[nx][ny].getType() == TileType::Rock) {
                                if (rand() % 4 == 0) tiles[nx][ny].setType(TileType::Grass);
                            }
                        }
                    }
                }
            }
        }
    }

    /*
    // Generate rocky areas
    int numRockAreas = rand() % 4 + 2; // 2-5 rock formations
    for (int rock = 0; rock < numRockAreas; rock++) {
        int centerX = rand() % rows;
        int centerY = rand() % columns;
        int rockSize = rand() % 4 + 2; // 2-5 radius
        
        // Create irregular rock formations
        for (int dx = -rockSize; dx <= rockSize; dx++) {
            for (int dy = -rockSize; dy <= rockSize; dy++) {
                int newX = centerX + dx;
                int newY = centerY + dy;
                
                // Check bounds
                if (newX >= 0 && newX < rows && newY >= 0 && newY < columns) {
                    // Only place rocks on grass (not in water)
                    if (tiles[newX][newY].getType() == TileType::Grass) {
                        float distance = sqrt(dx*dx + dy*dy);
                        if (distance <= rockSize + (rand() % 2)) {
                            tiles[newX][newY].setType(TileType::Rock);
                        }
                    }
                }
            }
        }
    } */
}


Grid::Grid(int height, int width, int cellSize) {
    this->cellSize = cellSize;
    rows = height / this->cellSize;
    columns = width / this->cellSize;
    tiles = vector<vector<Tile>>(rows, vector<Tile>(columns));
    initialize(35);
}

void Grid::draw() {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < columns; col++) {
            DrawRectangle(col * cellSize, row * cellSize, cellSize-1, cellSize-1, getColor(tiles[row][col]));
        }
    }
    
}