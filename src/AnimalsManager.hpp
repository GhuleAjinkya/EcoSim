#pragma once
#include <vector>
#include <random>
#include "Entity.hpp"
#include "Grid.hpp"
#include <map>
using namespace std;

bool posEquals(GridPos a, GridPos b) {
    return a.x==b.x && a.y==b.y;
}

class AnimalsManager {
    vector<Entity> entities;
    void updateHerbivore(Entity&, Grid&);
    void updateCarnivore(Entity&, Grid&);
    int nextID = 1;
    
    public:
        AnimalsManager() {}
        void spawnEntity(Species, int, int, Grid&);
        void update(Grid&);
        map<int, int> matingPairs;  
        map<int, int> lastReproduceAge;
        vector<Entity> getEntities() {return entities;}
        void spawnHerbivoreGroups(Grid&, int maxTotal);
        void spawnCarnivoreGroups(Grid&, int maxTotal);
};

void AnimalsManager::spawnEntity(Species species, int x, int y, Grid &grid) {
    entities.push_back(Entity(nextID, x, y, 100, species));
    grid.tiles[y][x].setOccupiedID(nextID);
    nextID++;
}

void AnimalsManager::spawnHerbivoreGroups(Grid& grid, int maxTotal) {
    if (maxTotal <= 0) return;
    int total = 0;
    int attempts = 0;
    const int maxAttempts = 2000;
    while (total < maxTotal && attempts < maxAttempts) {
        ++attempts;
        int groupSize = (rand() % 4) + 2; 
        if (groupSize > (maxTotal - total)) groupSize = maxTotal - total;

        int cx = rand() % grid.getColumns();
        int cy = rand() % grid.getRows();
        if (grid.tiles[cy][cx].getType() == TileType::Water) continue;
        if (grid.tiles[cy][cx].getOccupiedID() != -1) continue;

        spawnEntity(Species::Herbivore, cx, cy, grid);
        ++total;

        int placedAround = 1;
        int innerAttempts = 0;
        while (placedAround < groupSize && innerAttempts < 200 && total < maxTotal) {
            ++innerAttempts;
            int dx = (rand() % 5) - 2; 
            int dy = (rand() % 5) - 2;
            int nx = cx + dx;
            int ny = cy + dy;
            if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
            if (grid.tiles[ny][nx].getType() == TileType::Water || grid.tiles[ny][nx].getType() == TileType::Rock) continue;
            if (grid.tiles[ny][nx].getOccupiedID() != -1) continue;
            spawnEntity(Species::Herbivore, nx, ny, grid);
            ++placedAround;
            ++total;
        }
    }
}

void AnimalsManager::spawnCarnivoreGroups(Grid& grid, int maxTotal) {
    if (maxTotal <= 0) return;
    int total = 0;
    int attempts = 0;
    const int maxAttempts = 2000;
    while (total < maxTotal && attempts < maxAttempts) {
        ++attempts;

        int cx = rand() % grid.getColumns();
        int cy = rand() % grid.getRows();
        if (grid.tiles[cy][cx].getType() == TileType::Water) continue;
        if (grid.tiles[cy][cx].getOccupiedID() != -1) continue;

        spawnEntity(Species::Carnivore, cx, cy, grid);
        ++total;
    }
}

void AnimalsManager::updateHerbivore(Entity &entity, Grid& grid) {
    if (rand() % 5 == 0) entity.addHealth(-1);
    entity.tickAge();


    const int searchRadius = 5;
    bool foundFood = false;
    GridPos bestTarget = entity.getPos();
    float bestDistSq = float(searchRadius*searchRadius) + 0.0001;

    if (entity.getHealth() <= 75) {
        for (int dy = -searchRadius; dy <= searchRadius; ++dy) {
            for (int dx = -searchRadius; dx <= searchRadius; ++dx) {
                int nx = entity.getPos().x + dx;
                int ny = entity.getPos().y + dy;
                if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
                Tile &t = grid.tiles[ny][nx];
                if (t.getType() == TileType::Grass && t.getHealth() > 0) {
                    float dsq = float(dx*dx + dy*dy);
                    if (dsq < bestDistSq) {
                        bestDistSq = dsq;
                        bestTarget = {nx, ny};
                        foundFood = true;
                    }
                }
            }
        }
    }
    

    GridPos newPos = entity.getPos();
    if (foundFood) {
        if (bestTarget.x > entity.getPos().x) newPos.x++;
        if (bestTarget.x < entity.getPos().x) newPos.x--;
        if (bestTarget.y > entity.getPos().y) newPos.y++;
        if (bestTarget.y < entity.getPos().y) newPos.y--;
        foundFood = false;
    } else {
        if (entity.getMovementDelay()) {
            entity.tickMovementDelay();
        } else {
            if (rand() % 2 == 0) {
            int dx = (rand() % 3) - 1; 
            int dy = (rand() % 3) - 1;
            newPos.x += dx;
            newPos.y += dy;
            entity.setMovementDelay(30);
        }
        }
    }

    newPos.x = max(0, std::min(newPos.x, grid.getColumns()-1));
    newPos.y = max(0, std::min(newPos.y, grid.getRows()-1));

    Tile &targetTile = grid.tiles[newPos.y][newPos.x];
    if (targetTile.getType() != TileType::Water && targetTile.getOccupiedID() == -1) {
        // free previous tile occupancy
        GridPos old = entity.getPos();
        Tile &oldTile = grid.tiles[old.y][old.x];
        oldTile.freeOccupant();

        entity.setPos(newPos);
        targetTile.setOccupiedID(entity.getID());
    }

    if (entity.getHealth() <= 50) {
        Tile &here = grid.tiles[entity.getPos().y][entity.getPos().x];
        if (here.getType() == TileType::Grass) {
            here.setType(TileType::Rock);
            entity.addHealth(80);
        }
    }
    if (entity.getHealth() >= 75 && entity.getAge() > 500) {
        if ((rand() % 1000) < 2) { 
            int nearbyCount = 0;
            for (Entity &o : entities) {
                if (o.getSpecies() == entity.getSpecies()) {
                    int dx = o.getPos().x - entity.getPos().x;
                    int dy = o.getPos().y - entity.getPos().y;
                    if (abs(dx) <= 5 && abs(dy) <= 5) nearbyCount++;
                }
            }
            int maxNearby = 4;
            if (nearbyCount <= maxNearby) {
                bool spawned = false;
                for (int dy = -1; dy <= 1 && !spawned; ++dy) {
                    for (int dx = -1; dx <= 1 && !spawned; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int nx = entity.getPos().x + dx;
                        int ny = entity.getPos().y + dy;
                        if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
                        Tile &tt = grid.tiles[ny][nx];
                        if (tt.getType() == TileType::Water || tt.getType() == TileType::Rock) continue;
                        if (tt.getOccupiedID() != -1) continue;
                        int parentHealth = entity.getHealth();
                        int childHealth = max(1, parentHealth * 35 / 100);
                        entity.setHealth(parentHealth - childHealth);
                        spawnEntity(Species::Herbivore, nx, ny, grid);
                        tt.setOccupiedID(nextID);
                        nextID++;
                        spawned = true;
                    }
                }
            }
        }
    }
}

void AnimalsManager::updateCarnivore(Entity &entity, Grid& grid) {
    if (rand() % 5 == 0) entity.addHealth(-1); 
    entity.tickAge();

    const int searchRadius = 7;
    bool foundPrey = false;
    GridPos bestTarget = entity.getPos();
    float bestDistSq = float(searchRadius*searchRadius) + 0.0001f;

    // check if in mating phase
    bool inMatingPhase = false;
    int mateID = -1;
    if (matingPairs.find(entity.getID()) != matingPairs.end()) {
        mateID = matingPairs[entity.getID()];
        inMatingPhase = true;
    }
    Entity *mate = nullptr;

    if (!inMatingPhase) {
        // normal hunt behavior
        if (entity.getHealth() <= 60) {
            for (size_t i = 0; i < entities.size(); ++i) {
                Entity &other = entities[i];
                if (other.getSpecies() != Species::Herbivore) continue;
                
                int dx = other.getPos().x - entity.getPos().x;
                int dy = other.getPos().y - entity.getPos().y;
                float distSq = float(dx*dx + dy*dy);
                
                if (distSq < bestDistSq) {
                    bestDistSq = distSq;
                    bestTarget = other.getPos();
                    foundPrey = true;
                }
            }
        }

        // reproduction initiation: find nearest carnivore to mate with
        if (entity.getHealth() > 70 && entity.getAge() > 750) {
            int lastRepro = (lastReproduceAge.find(entity.getID()) != lastReproduceAge.end()) 
                            ? lastReproduceAge[entity.getID()] : -750;
            if (entity.getAge() - lastRepro >= 750) {
                float nearestMateDistSq = 999999.0f;
                int nearestMateID = -1;
                for (size_t i = 0; i < entities.size(); ++i) {
                    Entity &other = entities[i];
                    if (other.getSpecies() != Species::Carnivore || other.getID() == entity.getID()) continue;
                    if (other.getHealth() <= 70 || other.getAge() <= 750) continue;

                    int lastOtherRepro = (lastReproduceAge.find(other.getID()) != lastReproduceAge.end()) 
                                         ? lastReproduceAge[other.getID()] : -750;
                    if (other.getAge() - lastOtherRepro < 750) continue;

                    int dx = other.getPos().x - entity.getPos().x;
                    int dy = other.getPos().y - entity.getPos().y;
                    float distSq = float(dx*dx + dy*dy);
                    
                    if (distSq < nearestMateDistSq && matingPairs.find(other.getID()) == matingPairs.end()) {
                        nearestMateDistSq = distSq;
                        nearestMateID = other.getID();
                    }
                }

                if (nearestMateID != -1) {
                    matingPairs[entity.getID()] = nearestMateID;
                    matingPairs[nearestMateID] = entity.getID();
                    inMatingPhase = true;
                    mateID = nearestMateID;
                    foundPrey = false;
                }
            }
        }
    }

    // movement
    GridPos newPos = entity.getPos();
    if (inMatingPhase && mateID != -1) {
        // move toward mate
        for (int i = 0; i < entities.size(); ++i) {
            if (entities[i].getID() == mateID) {
                mate = &entities[i];
                break;
            }
        }

        if (mate) {
            if (mate->getPos().x > entity.getPos().x) newPos.x++;
            if (mate->getPos().x < entity.getPos().x) newPos.x--;
            if (mate->getPos().y > entity.getPos().y) newPos.y++;
            if (mate->getPos().y < entity.getPos().y) newPos.y--;
        }
    } else if (foundPrey) {
        if (bestTarget.x > entity.getPos().x) newPos.x++;
        if (bestTarget.x < entity.getPos().x) newPos.x--;
        if (bestTarget.y > entity.getPos().y) newPos.y++;
        if (bestTarget.y < entity.getPos().y) newPos.y--;
    } else {
        if (entity.getMovementDelay()) {
            entity.tickMovementDelay();
        } else {
            if (rand() % 2 == 0) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;
                newPos.x += dx;
                newPos.y += dy;
                entity.setMovementDelay(30);
            }
        }
    }

    newPos.x = max(0, min(newPos.x, grid.getColumns()-1));
    newPos.y = max(0, min(newPos.y, grid.getRows()-1));

    Tile &targetTile = grid.tiles[newPos.y][newPos.x];
    if (targetTile.getType() != TileType::Water && targetTile.getOccupiedID() == -1) {
        GridPos old = entity.getPos();
        Tile &oldTile = grid.tiles[old.y][old.x];
        oldTile.freeOccupant();

        entity.setPos(newPos);
        targetTile.setOccupiedID(entity.getID());
    }

    if (inMatingPhase && mateID != -1) {
        if (mate) {
            int dx = abs(mate->getPos().x - entity.getPos().x);
            int dy = abs(mate->getPos().y - entity.getPos().y);
            if (dx <= 3 && dy <= 3) {
                // spawn offspring at midpoint or nearby
                int spawnX = (entity.getPos().x + mate->getPos().x) / 2;
                int spawnY = (entity.getPos().y + mate->getPos().y) / 2;

                // find empty adjacent tile for offspring
                bool spawned = false;
                for (int sy = -1; sy <= 1 && !spawned; ++sy) {
                    for (int sx = -1; sx <= 1 && !spawned; ++sx) {
                        int nx = spawnX + sx;
                        int ny = spawnY + sy;
                        if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
                        Tile &tt = grid.tiles[ny][nx];
                        if (tt.getType() == TileType::Water || tt.getType() == TileType::Rock) continue;
                        if (tt.getOccupiedID() != -1) continue;

                        // create offspring with reduced health
                        spawnEntity(Species::Carnivore, nx, ny, grid);
                        tt.setOccupiedID(nextID - 1);

                        lastReproduceAge[entity.getID()] = entity.getAge();
                        lastReproduceAge[mateID] = mate->getAge();

                        spawned = true;
                    }
                }

                matingPairs.erase(entity.getID());
                matingPairs.erase(mateID);
            }
        }
    }

    if (!inMatingPhase && entity.getHealth() <= 40) {
        for (int i = 0; i < entities.size(); ++i) {
            Entity &other = entities[i];
            if (other.getSpecies() != Species::Herbivore) continue;
            
            int dx = abs(other.getPos().x - entity.getPos().x);
            int dy = abs(other.getPos().y - entity.getPos().y);
            if (dx <= 1 && dy <= 1) {
                other.addHealth(-150); 
                entity.addHealth(75); 
            }
        }
    }

    if (entity.getHealth() > 120) entity.setHealth(120);
}

void AnimalsManager::update(Grid& grid) {
    for (int i = 0; i < entities.size(); ++i) {
        Entity &e = entities[i];
        e.tickMovementDelay();
        if (e.getSpecies() == Species::Herbivore) {
            updateHerbivore(e, grid);
        }  else {
            updateCarnivore(e, grid);
        }
    }  
    entities.erase(remove_if(entities.begin(), entities.end(),
    [](Entity &e){ return e.getHealth() <= 0; }),
    entities.end());
}
