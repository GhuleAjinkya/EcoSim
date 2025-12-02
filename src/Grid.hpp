#pragma once
#include <vector>
#include <cstdlib>
#include <math.h>
#include "Tile.hpp"
using namespace std;

int seed = 70;
class Grid {
        int rows;
        int columns;
        int cellSize;
    public:
        vector<vector<Tile>> tiles;
        Grid(int, int, int);
        void initialize(); 
        void draw();
        int getRows() {return rows;}
        int getColumns() {return columns;}
        int getCellSize() {return cellSize;}
};

void Grid::initialize() {
    srand(Constants::GRID_SEED);
    
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
    int lakeSize = rand() % 6 + 6; 

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
    
    // Grass
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

}


Grid::Grid(int height, int width, int cellSize) {
    this->cellSize = cellSize;
    rows = height / this->cellSize;
    columns = width / this->cellSize;
    tiles = vector<vector<Tile>>(rows, vector<Tile>(columns));
    initialize();
}

void Grid::draw() {
    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < columns; col++) {
            DrawRectangle(col * cellSize, row * cellSize, cellSize-1, cellSize-1, getColor(tiles[row][col]));
        }
    }
    
}