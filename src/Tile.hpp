#include <raylib.h>

int decayCooldown = 12;

enum TileType {
    Grass,
    Water,
    Rock
};

class Tile {
    TileType type;
    int occupiedID = -1;
    bool isOccupied = false;
    int health = -1;
    public:
        int getOccupiedID();
        void setOccupiedID(int id) {isOccupied = true;occupiedID = id;}
        void freeOccupant() {isOccupied = false;}
        TileType getType() {return type;}
        void setType(TileType tType);
        void update(int surrGrass);
        int getHealth() {return health;}
};

int Tile::getOccupiedID() {
    if (isOccupied) return occupiedID;
    return -1;
}

Color getColor(Tile tile) {
    if (tile.getType() == TileType::Grass) return {21, 191, 64, 255};
    if (tile.getType() == TileType::Water) return {0, 0, 255, 255};
    if (tile.getType() == TileType::Rock) return {76, 50, 33, 255};
}

void Tile::setType(TileType tType) {
    if (tType == TileType::Grass) {
        health = 100;
    }
    if (tType == TileType::Rock) {
        health = 0;
    }
    type = tType;
}

void Tile::update(int surrGrass) {
    if (getType() == TileType::Water) return;
    if (getType() == TileType::Rock) {
        if (rand() % 3 == 0) health += surrGrass * 1;
        if (health >= 100) setType(TileType::Grass);
    }
    if (getType() == TileType::Grass) {
        if (rand() % 100 == 0) health -= 5;
        if (health <= 0) {
            if (!decayCooldown) {
                setType(TileType::Rock);
                decayCooldown = 12;
            }
            else decayCooldown--;

        }
    }
}