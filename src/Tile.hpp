#include <raylib.h>

enum TileType {
    Grass,
    Water,
    Rock
};

class Tile {
    TileType type;
    int occupiedID;
    bool isOccupied;
    int health = -1;
    public:
        int getOccupiedID() {return occupiedID;}
        void setOccupiedID(int id) {occupiedID = id;}
        TileType getType() {return type;}
        void setType(TileType tType);
        void update(int surrGrass);
        int getHealth() {return health;}
};

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
        if (rand() % 5 == 0) health += surrGrass * 3;
        if (health >= 100) setType(TileType::Grass);
    }
    if (getType() == TileType::Grass) {
        if (rand() % 20 == 0) health -= 3;
        if (health <= 0) setType(TileType::Rock);
    }
}