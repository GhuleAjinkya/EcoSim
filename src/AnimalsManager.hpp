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

class AnimalsManager : public Entity{
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
    while (total < maxTotal && attempts < Constants::SPAWN_MAX_ATTEMPTS) {
        attempts++;
        int range = Constants::HERBIVORE_SPAWN_MAX_GROUP_SIZE - Constants::HERBIVORE_SPAWN_MIN_GROUP_SIZE + 1;
        int groupSize = (rand() % range) + Constants::HERBIVORE_SPAWN_MIN_GROUP_SIZE;
        if (groupSize > (maxTotal - total)) groupSize = maxTotal - total;

        int cx = rand() % grid.getColumns();
        int cy = rand() % grid.getRows();
        if (grid.tiles[cy][cx].getType() == TileType::Water) continue;
        if (grid.tiles[cy][cx].getOccupiedID() != -1) continue;

        spawnEntity(Species::Herbivore, cx, cy, grid);
        total++;

        int placedAround = 1;
        int innerAttempts = 0;
        while (placedAround < groupSize && innerAttempts < Constants::SPAWN_INNER_ATTEMPTS && total < maxTotal) {
            innerAttempts++;
            int dx = (rand() % (Constants::HERBIVORE_SPAWN_CLUSTER_RADIUS * 2 + 1)) - Constants::HERBIVORE_SPAWN_CLUSTER_RADIUS;
            int dy = (rand() % (Constants::HERBIVORE_SPAWN_CLUSTER_RADIUS * 2 + 1)) - Constants::HERBIVORE_SPAWN_CLUSTER_RADIUS;
            int nx = cx + dx;
            int ny = cy + dy;
            if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
            if (grid.tiles[ny][nx].getType() == TileType::Water || grid.tiles[ny][nx].getType() == TileType::Rock) continue;
            if (grid.tiles[ny][nx].getOccupiedID() != -1) continue;
            spawnEntity(Species::Herbivore, nx, ny, grid);
            placedAround++;
            total++;
        }
    }
}

void AnimalsManager::spawnCarnivoreGroups(Grid& grid, int maxTotal) {
    if (maxTotal <= 0) return;
    int total = 0;
    int attempts = 0;
    while (total < maxTotal && attempts < Constants::SPAWN_MAX_ATTEMPTS) {
        attempts++;

        int cx = rand() % grid.getColumns();
        int cy = rand() % grid.getRows();
        if (grid.tiles[cy][cx].getType() == TileType::Water) continue;
        if (grid.tiles[cy][cx].getOccupiedID() != -1) continue;

        spawnEntity(Species::Carnivore, cx, cy, grid);
        total++;
    }
}

void AnimalsManager::updateHerbivore(Entity &entity, Grid& grid) {
    if (Constants::HERBIVORE_DECAY_CHANCE > 0) {
        if (rand() % (100 / Constants::HERBIVORE_DECAY_CHANCE) == 0) entity.addHealth(-Constants::HERBIVORE_HEALTH_DECAY);
    }
    entity.tickAge();

    const int searchRadius = Constants::HERBIVORE_SEARCH_RADIUS;
    bool foundFood = false;
    GridPos bestTarget = entity.getPos();
    float bestDistSq = float(searchRadius*searchRadius) + 0.0001f;

    if (entity.getHealth() <= Constants::HERBIVORE_HUNT_HEALTH_THRESHOLD) {
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
            if (Constants::HERBIVORE_WANDER_CHANCE > 0 && rand() % (100 / Constants::HERBIVORE_WANDER_CHANCE) == 0) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;
                newPos.x += dx;
                newPos.y += dy;
                entity.setMovementDelay(Constants::HERBIVORE_MOVEMENT_DELAY);
            }
        }
    }

    newPos.x = max(0, std::min(newPos.x, grid.getColumns()-1));
    newPos.y = max(0, std::min(newPos.y, grid.getRows()-1));

    Tile &targetTile = grid.tiles[newPos.y][newPos.x];
    if (targetTile.getType() != TileType::Water && targetTile.getOccupiedID() == -1) {
        GridPos old = entity.getPos();
        Tile &oldTile = grid.tiles[old.y][old.x];
        oldTile.freeOccupant();

        entity.setPos(newPos);
        targetTile.setOccupiedID(entity.getID());
    }

    if (entity.getHealth() <= Constants::HERBIVORE_FEED_HEALTH_THRESHOLD) {
        Tile &here = grid.tiles[entity.getPos().y][entity.getPos().x];
        if (here.getType() == TileType::Grass) {
            here.setType(TileType::Rock);
            entity.addHealth(Constants::HERBIVORE_FEED_GAIN);
        }
    }
    if (entity.getHealth() >= Constants::HERBIVORE_REPRODUCTION_HEALTH && entity.getAge() > Constants::HERBIVORE_REPRODUCTION_AGE) {
        if ((rand() % 1000) < Constants::HERBIVORE_REPRODUCTION_CHANCE) {
            int nearbyCount = 0;
            for (Entity &o : entities) {
                if (o.getSpecies() == entity.getSpecies()) {
                    int dx = o.getPos().x - entity.getPos().x;
                    int dy = o.getPos().y - entity.getPos().y;
                    if (abs(dx) <= 5 && abs(dy) <= 5) nearbyCount++;
                }
            }
            if (nearbyCount <= Constants::HERBIVORE_MAX_NEARBY_TO_REPRODUCE) {
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
                        int childHealth = max(1, parentHealth * Constants::HERBIVORE_CHILD_HEALTH_PCT / 100);
                        entity.setHealth(parentHealth - childHealth);
                        spawnEntity(Species::Herbivore, nx, ny, grid);
                        spawned = true;
                    }
                }
            }
        }
    }
}

void AnimalsManager::updateCarnivore(Entity &entity, Grid& grid) {
    if (Constants::CARNIVORE_DECAY_CHANCE > 0) {
        if (rand() % (100 / Constants::CARNIVORE_DECAY_CHANCE) == 0) entity.addHealth(-Constants::CARNIVORE_HEALTH_DECAY);
    }
    entity.tickAge();

    const int searchRadius = Constants::CARNIVORE_SEARCH_RADIUS;
    bool foundPrey = false;
    GridPos bestTarget = entity.getPos();
    float bestDistSq = float(searchRadius*searchRadius) + 0.0001f;

    bool inMatingPhase = false;
    int mateID = -1;
    if (matingPairs.find(entity.getID()) != matingPairs.end()) {
        mateID = matingPairs[entity.getID()];
        inMatingPhase = true;
    }
    Entity *mate = nullptr;

    if (!inMatingPhase) {
        if (entity.getHealth() <= Constants::CARNIVORE_HUNT_HEALTH_THRESHOLD) {
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

        if (entity.getHealth() > Constants::CARNIVORE_MATE_HEALTH && entity.getAge() > Constants::CARNIVORE_MATE_AGE) {
            int lastRepro = (lastReproduceAge.find(entity.getID()) != lastReproduceAge.end())
                            ? lastReproduceAge[entity.getID()] : -Constants::CARNIVORE_MATE_COOLDOWN;
            if (entity.getAge() - lastRepro >= Constants::CARNIVORE_MATE_COOLDOWN) {
                float nearestMateDistSq = 999999.0f;
                int nearestMateID = -1;
                for (size_t i = 0; i < entities.size(); ++i) {
                    Entity &other = entities[i];
                    if (other.getSpecies() != Species::Carnivore || other.getID() == entity.getID()) continue;
                    if (other.getHealth() <= Constants::CARNIVORE_MATE_HEALTH || other.getAge() <= Constants::CARNIVORE_MATE_AGE) continue;

                    int lastOtherRepro = (lastReproduceAge.find(other.getID()) != lastReproduceAge.end())
                                         ? lastReproduceAge[other.getID()] : -Constants::CARNIVORE_MATE_COOLDOWN;
                    if (other.getAge() - lastOtherRepro < Constants::CARNIVORE_MATE_COOLDOWN) continue;

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

    GridPos newPos = entity.getPos();
    if (inMatingPhase && mateID != -1) {
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
            if (Constants::CARNIVORE_WANDER_CHANCE > 0 && rand() % (100 / Constants::CARNIVORE_WANDER_CHANCE) == 0) {
                int dx = (rand() % 3) - 1;
                int dy = (rand() % 3) - 1;
                newPos.x += dx;
                newPos.y += dy;
                entity.setMovementDelay(Constants::CARNIVORE_MOVEMENT_DELAY);
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
            if (dx <= Constants::CARNIVORE_MATE_PROXIMITY && dy <= Constants::CARNIVORE_MATE_PROXIMITY) {
                int spawnX = (entity.getPos().x + mate->getPos().x) / 2;
                int spawnY = (entity.getPos().y + mate->getPos().y) / 2;

                bool spawned = false;
                for (int sy = -1; sy <= 1 && !spawned; ++sy) {
                    for (int sx = -1; sx <= 1 && !spawned; ++sx) {
                        int nx = spawnX + sx;
                        int ny = spawnY + sy;
                        if (nx < 0 || ny < 0 || nx >= grid.getColumns() || ny >= grid.getRows()) continue;
                        Tile &tt = grid.tiles[ny][nx];
                        if (tt.getType() == TileType::Water || tt.getType() == TileType::Rock) continue;
                        if (tt.getOccupiedID() != -1) continue;

                        spawnEntity(Species::Carnivore, nx, ny, grid);

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

    if (!inMatingPhase && entity.getHealth() <= Constants::CARNIVORE_ATTACK_HEALTH_THRESHOLD) {
        for (int i = 0; i < entities.size(); ++i) {
            Entity &other = entities[i];
            if (other.getSpecies() != Species::Herbivore) continue;

            int dx = abs(other.getPos().x - entity.getPos().x);
            int dy = abs(other.getPos().y - entity.getPos().y);
            if (dx <= 1 && dy <= 1) {
                other.addHealth(-Constants::CARNIVORE_ATTACK_DAMAGE);
                entity.addHealth(Constants::CARNIVORE_ATTACK_GAIN);
            }
        }
    }

    if (entity.getHealth() > Constants::CARNIVORE_MAX_HEALTH) entity.setHealth(Constants::CARNIVORE_MAX_HEALTH);
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
