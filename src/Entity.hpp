#pragma once
#include <raylib.h>
#include <algorithm>
#include "Constants.hpp"

struct GridPos { int x = 0; int y = 0; };
enum class Species {Herbivore, Carnivore};
class Entity {
    int id;
    int health;
    int age = 0;
    GridPos pos;
    Species species;
    bool canReproduce = false;
    int movementDelay = 50;
    static int herbivoreCount;
    static int carnivoreCount;
    public:
        Entity(int setID, int x, int y, int setHealth, Species setSpecies) 
        : pos{x,y}, health(setHealth), id(setID), species(setSpecies) {
            if (species == Species::Carnivore) {
                carnivoreCount++;
            } else {
                herbivoreCount++;
            }
        }
        Entity() {};
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

int Entity::herbivoreCount = 0;
int Entity::carnivoreCount = 0;

void Entity::tickMovementDelay() {
    if (movementDelay > 0) movementDelay--;
}

Color Entity::getColor() {
    if (species == Species::Carnivore) return {255,0,0,255};
    if (species == Species::Herbivore) return {255,255,0,255};
}

void Entity::addHealth(int val) {
    health += val;
    int maxHealth = (species == Species::Herbivore) ? 
        Constants::HERBIVORE_MAX_HEALTH 
        : Constants::CARNIVORE_MAX_HEALTH;
    if (health < 0) health = 0;
    if (health > maxHealth) health = maxHealth;
}


