#pragma once
#include <raylib.h>
#include <algorithm>
struct GridPos { int x = 0; int y = 0; };
enum class Species {Herbivore, Carnivore};
class Entity {
    int id;
    int health;
    int age = 0;
    GridPos pos;
    Species species;
    bool canReproduce = false;
    int movementDelay = 30;
    public:
        Entity(int setID, int x, int y, int setHealth, Species setSpecies) 
        : pos{x,y}, health(setHealth), id(setID), species(setSpecies) {}
        ~Entity() {};
        int getID() {return id;}
        Color getColor();
        GridPos getPos() {return pos;}
        void setPos(GridPos newPos) {pos = newPos;}
        Species getSpecies() {return species;}
        void addHealth(int val);
        void tickAge() {age++;}
        int getAge() {return age;}
        int getHealth() {return health;}
        int getMovementDelay() {return movementDelay;}
        void tickMovementDelay();
        void setMovementDelay(int val) {movementDelay = val;}
        void setHealth(int val) {health = val;}
};

void Entity::tickMovementDelay() {
    if (movementDelay > 0) movementDelay--;
}

Color Entity::getColor() {
    if (species == Species::Carnivore) return {255,0,0,255};
    if (species == Species::Herbivore) return {255,255,0,255};
}

void Entity::addHealth(int val) {
    health += val;
    // clamp to valid range [0, maxHealth] to prevent overflow/underflow
    int maxHealth = (species == Species::Herbivore) ? 100 : 120;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;
}


