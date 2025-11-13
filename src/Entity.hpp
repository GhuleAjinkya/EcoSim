#pragma once
#include <raylib.h>

struct GridPos { int x = 0; int y = 0; };

class Entity {
public:
    Entity(int x=0, int y=0) : pos{x,y} {}
    virtual ~Entity() = default;
    virtual void update(float dt) = 0;
    // draw using tileSize to convert grid->pixel coordinates
    virtual void draw(int tileSize) = 0;
    GridPos pos;
};
