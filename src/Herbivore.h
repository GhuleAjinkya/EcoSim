#pragma once
#include "Entity.hpp"
#include <raylib.h>

class Herbivore : public Entity {
public:
    Herbivore(int x, int y);
    void update(float dt) override;
    void draw(int tileSize) override;
private:
    Color col;
};
