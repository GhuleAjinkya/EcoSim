// src/World.cpp
/*
#include "World.h"
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics.hpp> // for fonts/textures used in UI

// small file-scope caches for visual smoothing / orientation hints
static std::unordered_map<int, sf::Vector2i> s_lastGridPos;
static std::unordered_map<int, sf::Vector2i> s_prevPos;
static sf::Clock s_visualClock;

// interpolation timing (set on update)
static sf::Clock s_lastUpdateClock;
static float s_lastDt = 0.016f;

// UI / selection state
static bool s_mouseDownPrev = false;
static int s_selectedId = -1;

// font + text resources (lazy loaded)
static sf::Font s_uiFont;
static bool s_uiFontLoaded = false;
static const std::string s_fontPath = "c:\\Users\\athar\\OneDrive\\Desktop\\Eco_sim\\assets\\fonts\\OpenSans-Regular.ttf";

// view management
static bool s_hasCustomView = false;
static sf::View s_worldView;

// Implementation of missing functions
void World::stepPlants(float dt) {
    for (auto& entity : m_entities) {
        if (entity.species != Species::Plant) continue;
        
        // Basic plant growth
        if (entity.energy < 100.0f) {
            const Tile& tile = m_tiles[entity.pos.y * m_width + entity.pos.x];
            entity.energy += dt * tile.fertility * 10.0f;
        }
        
        entity.age += 1;
    }
}

void World::stepAnimals(float dt) {
    for (auto& entity : m_entities) {
        if (entity.species == Species::Plant) continue;
        
        // Very low base energy consumption
        float consumptionRate = (entity.species == Species::Herbivore) ? 1.0f : 1.5f;
        // Additional energy cost when moving
        bool moved = false;

        // Feed when close to food and gain energy
        for (auto& other : m_entities) {
            if ((entity.species == Species::Herbivore && other.species == Species::Plant && other.energy > 0.5f) ||
                (entity.species == Species::Carnivore && other.species == Species::Herbivore)) {
                
                int dx = other.pos.x - entity.pos.x;
                int dy = other.pos.y - entity.pos.y;
                if (abs(dx) <= 1 && abs(dy) <= 1) {  // If adjacent to food
                    // More energy gain from feeding
                    entity.energy += dt * 25.0f;  // Increased energy gain
                    
                    // Reduce prey's energy/health
                    if (entity.species == Species::Carnivore) {
                        other.energy -= dt * 30.0f; // Carnivores damage herbivores faster
                    } else if (entity.species == Species::Herbivore) {
                        other.energy -= dt * 5.0f;  // Herbivores slowly consume plants
                    }
                    
                    // Reproduction chance when well-fed
                    float maxEnergy = (entity.species == Species::Herbivore) ? 60.0f : 120.0f;
                    
                    // Different reproduction rules for herbivores and carnivores
                    bool canReproduce = false;
                    if (entity.species == Species::Herbivore) {
                        // Herbivores need more energy and age requirements to reproduce
                        canReproduce = entity.energy > maxEnergy * 0.9f && // Need 90% energy
                                     entity.age > 100 && // Must be mature
                                     m_d01(m_rng) < dt * 0.02f; // Lower reproduction chance
                    } else {
                        // Carnivores keep original rules
                        canReproduce = entity.energy > maxEnergy * 0.8f && 
                                     m_d01(m_rng) < dt * 0.1f;
                    }
                    
                    if (canReproduce) {
                        // Count nearby same species to prevent overcrowding
                        int nearbyCount = 0;
                        for (const auto& other : m_entities) {
                            if (other.species == entity.species) {
                                int dx = other.pos.x - entity.pos.x;
                                int dy = other.pos.y - entity.pos.y;
                                if (abs(dx) <= 3 && abs(dy) <= 3) { // Check 3x3 area
                                    nearbyCount++;
                                }
                            }
                        }
                        
                        // Only reproduce if not too crowded
                        int maxNearby = (entity.species == Species::Herbivore) ? 4 : 2;
                        if (nearbyCount <= maxNearby) {
                            Entity newEntity;
                            newEntity.id = m_nextId++;
                            newEntity.species = entity.species;
                            newEntity.pos = entity.pos;
                            newEntity.energy = entity.energy * 0.35f;  // Parent gives 35% energy to child
                            entity.energy *= 0.65f;  // Parent keeps 65% energy
                            newEntity.age = 0;
                            m_entities.push_back(newEntity);
                        }
                    }
                    
                    // Cap maximum energy
                    if (entity.energy > maxEnergy) {
                        entity.energy = maxEnergy;
                    }
                }
            }
        }

        // Movement logic
        // Define search radius for food
        const int searchRadius = (entity.species == Species::Herbivore) ? 5 : 7;
        
        // Find nearest food
        sf::Vector2i targetPos = entity.pos;
        float closestDist = (float)(searchRadius * searchRadius);
        bool foundFood = false;

        // Search for food in radius
        for (const auto& other : m_entities) {
            // Herbivores look for plants, carnivores look for herbivores
            if ((entity.species == Species::Herbivore && other.species == Species::Plant) ||
                (entity.species == Species::Carnivore && other.species == Species::Herbivore)) {
                
                int dx = other.pos.x - entity.pos.x;
                int dy = other.pos.y - entity.pos.y;
                float distSq = (float)(dx * dx + dy * dy);
                
                if (distSq < closestDist) {
                    targetPos = other.pos;
                    closestDist = distSq;
                    foundFood = true;
                }
            }
        }

        // Decide movement
        sf::Vector2i newPos = entity.pos;
        if (foundFood) {
            // Move towards food
            if (targetPos.x > entity.pos.x) newPos.x++;
            if (targetPos.x < entity.pos.x) newPos.x--;
            if (targetPos.y > entity.pos.y) newPos.y++;
            if (targetPos.y < entity.pos.y) newPos.y--;
        } else {
            // Random movement with 40% chance and more dynamic movement
            if (m_d01(m_rng) < 0.4f) {
                // More dynamic random movement
                int dx = (int)(m_d01(m_rng) * 3.0f) - 1; // -1, 0, or 1
                int dy = (int)(m_d01(m_rng) * 3.0f) - 1; // -1, 0, or 1
                newPos.x += dx;
                newPos.y += dy;
                
                // Extra movement chance when energy is high
                if (entity.energy > (entity.species == Species::Herbivore ? 40.0f : 80.0f) && 
                    m_d01(m_rng) < 0.3f) {
                    newPos.x += dx;
                    newPos.y += dy;
                }
            }
        }

        // Validate new position
        if (newPos.x >= 0 && newPos.x < m_width && 
            newPos.y >= 0 && newPos.y < m_height) {
            // Check if the new position is not water or rock
            const Tile& tile = m_tiles[newPos.y * m_width + newPos.x];
            if (tile.type != TileType::Water && tile.type != TileType::Rock) {
                // Check if position is not occupied by another entity
                bool occupied = false;
                for (const auto& other : m_entities) {
                    if (other.id != entity.id && other.pos == newPos) {
                        occupied = true;
                        break;
                    }
                }
                if (!occupied) {
                    entity.pos = newPos;
                }
            }
        }

        // If energy is too low, entity dies
        if (entity.energy <= 0) {
            entity.energy = 0; // Will be removed in update loop
        }
    }

    // Remove dead entities
    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(),
            [](const Entity& e) { return e.energy <= 0; }
        ),
        m_entities.end()
    );
}

sf::Vector2f World::toPixel(const sf::Vector2i& g) const {
    return sf::Vector2f(g.x * m_tileSize, g.y * m_tileSize);
}

sf::Color World::colorForTile(const Tile& t) const {
    switch (t.type) {
        case TileType::Water:
            return sf::Color(0, 0, 255);
        case TileType::Rock:
            return sf::Color(128, 128, 128);
        case TileType::Fertile:
            return sf::Color(139, 69, 19);
        case TileType::Grass:
        default:
            return sf::Color(0, 255 * t.moisture, 0);
    }
}

int World::countSpecies(Species s) const {
    int count = 0;
    for (const auto& entity : m_entities) {
        if (entity.species == s) {
            count++;
        }
    }
    return count;
}

World::World(int width, int height, int tileSize)
: m_width(width), m_height(height), m_tileSize(tileSize),
  m_tileVerts(sf::Quads), m_rng(std::random_device{}()), m_d01(0.f,1.f), m_nextId(1)
{
    m_tiles.resize(m_width * m_height);
    // simple terrain
    for (int y=0;y<m_height;++y) for (int x=0;x<m_width;++x) {
        float r = m_d01(m_rng);
        Tile &t = m_tiles[x + y*m_width];
        if (r < 0.06f) { t.type = TileType::Water; t.fertility = 0.2f; t.moisture = 1.f; }
        else if (r < 0.12f) { t.type = TileType::Rock; t.fertility = 0.f; t.moisture = 0.f; }
        else if (r < 0.3f) { t.type = TileType::Fertile; t.fertility = 1.f; t.moisture = 0.8f; }
        else { t.type = TileType::Grass; t.fertility = 0.6f + 0.4f*m_d01(m_rng); t.moisture = 0.4f + 0.5f*m_d01(m_rng); }
    }
    // spawn some initial entities
    for (int i=0;i<800;++i) { // plants
        int x = int(m_d01(m_rng) * m_width), y = int(m_d01(m_rng)*m_height);
        if (m_tiles[x + y*m_width].type != TileType::Rock) {
            Entity e; e.id = m_nextId++; e.species = Species::Plant; e.pos = {x,y}; e.energy = 1.f; e.age=0;
            m_entities.push_back(e);
        }
    }
    for (int i=0;i<80;++i) {
        int x = int(m_d01(m_rng) * m_width), y = int(m_d01(m_rng)*m_height);
        if (m_tiles[x + y*m_width].type != TileType::Rock) {
            Entity e; e.id = m_nextId++; e.species = Species::Herbivore; e.pos = {x,y}; e.energy = 20.f; e.age=0;
            m_entities.push_back(e);
        }
    }
    for (int i=0;i<20;++i) {
        int x = int(m_d01(m_rng) * m_width), y = int(m_d01(m_rng)*m_height);
        if (m_tiles[x + y*m_width].type != TileType::Rock) {
            Entity e; e.id = m_nextId++; e.species = Species::Carnivore; e.pos = {x,y}; e.energy = 30.f; e.age=0;
            m_entities.push_back(e);
        }
    }

    rebuildVertexArray();
}

void World::reset() {
    m_tiles.clear(); m_entities.clear(); m_nextId = 1;
    m_tiles.resize(m_width * m_height);
    // regenerate terrain quickly
    for (int y=0;y<m_height;++y) for (int x=0;x<m_width;++x) {
        float r = m_d01(m_rng);
        Tile &t = m_tiles[x + y*m_width];
        if (r < 0.06f) { t.type = TileType::Water; t.fertility = 0.2f; t.moisture = 1.f; }
        else if (r < 0.12f) { t.type = TileType::Rock; t.fertility = 0.f; t.moisture = 0.f; }
        else if (r < 0.3f) { t.type = TileType::Fertile; t.fertility = 1.f; t.moisture = 0.8f; }
        else { t.type = TileType::Grass; t.fertility = 0.6f + 0.4f*m_d01(m_rng); t.moisture = 0.4f + 0.5f*m_d01(m_rng); }
    }
    // re-add some entities
    for (int i=0;i<600;++i) {
        int x = int(m_d01(m_rng) * m_width), y = int(m_d01(m_rng)*m_height);
        if (m_tiles[x + y*m_width].type != TileType::Rock) {
            Entity e; e.id = m_nextId++; e.species = Species::Plant; e.pos = {x,y}; e.energy = 1.f; e.age=0;
            m_entities.push_back(e);
        }
    }
    rebuildVertexArray();
}

void World::update(float dt) {
    // snapshot previous positions for interpolation
    s_prevPos.clear();
    for (auto &e : m_entities) s_prevPos[e.id] = e.pos;
    s_lastDt = dt > 0.0001f ? dt : 0.016f;
    s_lastUpdateClock.restart();

    stepPlants(dt);
    stepAnimals(dt);
    rebuildVertexArray();
}

void World::draw(sf::RenderWindow& target) {
    // lazy-load font
    if (!s_uiFontLoaded) {
        s_uiFontLoaded = s_uiFont.loadFromFile(s_fontPath);
        // if load fails, s_uiFontLoaded remains false and text will be skipped
    }

    // save default view for UI drawing later
    sf::View defaultView = target.getDefaultView();

    // mouse positions (window pixels)
    sf::Vector2i mousePixel = sf::Mouse::getPosition(target);

    // map mouse -> world coords using the active world view (if set) so hover works while panned
    sf::Vector2f worldMouse;
    if (s_hasCustomView) worldMouse = target.mapPixelToCoords(mousePixel, s_worldView);
    else worldMouse = target.mapPixelToCoords(mousePixel, defaultView);

    sf::Vector2i mouseGrid((int)(worldMouse.x / m_tileSize), (int)(worldMouse.y / m_tileSize));
    int hoveredEntityIdx = -1;
    if (mouseGrid.x >= 0 && mouseGrid.y >= 0 && mouseGrid.x < m_width && mouseGrid.y < m_height) {
        hoveredEntityIdx = entityIndexAt(mouseGrid);
    }

    // --- world rendering ---
    if (s_hasCustomView) {
        // ensure world view size matches window (in world pixels)
        s_worldView.setSize(defaultView.getSize());
        target.setView(s_worldView);
    } else {
        target.setView(defaultView);
    }

    // draw tiles (respecting current view)
    sf::RenderStates states;
    target.draw(m_tileVerts, states);

    // pre-create reusable shapes to avoid allocations in the loop
    float globalT = s_visualClock.getElapsedTime().asSeconds();
    float interpT = 1.f;
    if (s_lastDt > 0.0001f) interpT = std::min(1.f, s_lastUpdateClock.getElapsedTime().asSeconds() / s_lastDt);

    sf::CircleShape circle((float)m_tileSize * 0.4f);
    circle.setOrigin(circle.getRadius(), circle.getRadius());
    sf::RectangleShape rect(sf::Vector2f((float)m_tileSize*0.8f, (float)m_tileSize*0.8f));
    rect.setOrigin(rect.getSize()/2.f);

    for (auto &e : m_entities) {
        // interpolation: previous grid pos -> current grid pos
        sf::Vector2i prevG = s_prevPos.count(e.id) ? s_prevPos[e.id] : e.pos;
        sf::Vector2f pPrev = toPixel(prevG);
        sf::Vector2f pNow  = toPixel(e.pos);
        sf::Vector2f p = pPrev + (pNow - pPrev) * interpT;

        // small bobbing for life (uses id to vary phase)
        float bob = std::sin(globalT * 3.0f + (float)e.id * 0.37f) * ((float)m_tileSize * 0.04f);
        p.y += bob;

        float shadowOffsetY = (float)m_tileSize * 0.06f;
        if (e.species == Species::Plant) {
            float size = (float)m_tileSize * (0.25f + 0.18f * std::clamp(e.energy / 5.f, 0.f, 1.f));
            circle.setRadius(size);
            circle.setOrigin(size, size);

            sf::CircleShape shadow(size);
            shadow.setOrigin(size, size);
            shadow.setPosition(p + sf::Vector2f(3.f, shadowOffsetY+3.f));
            shadow.setFillColor(sf::Color(0,0,0,90));
            target.draw(shadow);

            sf::Color base(30,160,30);
            float en = std::clamp(e.energy / 5.f, 0.f, 1.f);
            sf::Color fill((sf::Uint8)(base.r * (0.5f + 0.5f*en)),
                           (sf::Uint8)(base.g * (0.6f + 0.4f*en)),
                           (sf::Uint8)(base.b * (0.5f + 0.5f*en)));
            circle.setPosition(p);
            circle.setFillColor(fill);
            circle.setOutlineColor(sf::Color((sf::Uint8)(fill.r*0.45f),(sf::Uint8)(fill.g*0.45f),(sf::Uint8)(fill.b*0.45f)));
            circle.setOutlineThickness(1.f);
            target.draw(circle);
        } else if (e.species == Species::Herbivore) {
            sf::Vector2i last = s_lastGridPos.count(e.id) ? s_lastGridPos[e.id] : e.pos;
            sf::Vector2i mv = sf::Vector2i(e.pos.x - last.x, e.pos.y - last.y);
            float angle = 0.f;
            if (mv.x != 0 || mv.y != 0) angle = std::atan2((float)mv.y, (float)mv.x) * 180.f / 3.14159f;

            float sizeX = (float)m_tileSize * (0.75f);
            float sizeY = (float)m_tileSize * (0.55f);
            rect.setSize({sizeX, sizeY});
            rect.setOrigin(rect.getSize()/2.f);

            sf::RectangleShape shadow(rect.getSize());
            shadow.setOrigin(rect.getOrigin());
            shadow.setPosition(p + sf::Vector2f(4.f, shadowOffsetY+4.f));
            shadow.setFillColor(sf::Color(0,0,0,100));
            shadow.setRotation(angle);
            target.draw(shadow);

            float maxEnergy = 60.f;
            float en = std::clamp(e.energy / maxEnergy, 0.f, 1.f);
            sf::Color base(200,200,50);
            sf::Color fill((sf::Uint8)(base.r * (0.5f + 0.5f*en)),
                           (sf::Uint8)(base.g * (0.5f + 0.6f*en)),
                           (sf::Uint8)(base.b * (0.4f + 0.6f*en)));
            rect.setPosition(p);
            rect.setRotation(angle);
            rect.setFillColor(fill);
            rect.setOutlineColor(sf::Color((sf::Uint8)(fill.r*0.4f),(sf::Uint8)(fill.g*0.4f),(sf::Uint8)(fill.b*0.4f)));
            rect.setOutlineThickness(1.f);
            target.draw(rect);
        } else {
            float size = (float)m_tileSize * (0.45f + 0.15f * std::clamp(e.energy / 120.f, 0.f, 1.f));
            circle.setRadius(size);
            circle.setOrigin(size, size);

            sf::CircleShape shadow(size);
            shadow.setOrigin(size, size);
            shadow.setPosition(p + sf::Vector2f(4.f, shadowOffsetY+4.f));
            shadow.setFillColor(sf::Color(0,0,0,110));
            target.draw(shadow);

            float maxEnergy = 120.f;
            float en = std::clamp(e.energy / maxEnergy, 0.f, 1.f);
            sf::Color base(200,80,80);
            sf::Color fill((sf::Uint8)(base.r * (0.5f + 0.5f*en)),
                           (sf::Uint8)(base.g * (0.4f + 0.5f*en)),
                           (sf::Uint8)(base.b * (0.4f + 0.5f*en)));
            circle.setPosition(p);
            circle.setFillColor(fill);
            circle.setOutlineColor(sf::Color((sf::Uint8)(fill.r*0.35f),(sf::Uint8)(fill.g*0.35f),(sf::Uint8)(fill.b*0.35f)));
            circle.setOutlineThickness(1.f);
            target.draw(circle);
        }

        // energy bar
        float cap = (e.species == Species::Plant ? 5.f : (e.species == Species::Herbivore ? 60.f : 120.f));
        float norm = std::clamp(e.energy / cap, 0.f, 1.f);
        float barW = (float)m_tileSize * 0.7f;
        float barH = (float)m_tileSize * 0.09f;
        sf::RectangleShape barBg(sf::Vector2f(barW, barH));
        barBg.setOrigin(barW/2.f, barH/2.f);
        barBg.setPosition(p.x, p.y - (float)m_tileSize*0.6f);
        barBg.setFillColor(sf::Color(30,30,30,180));
        barBg.setOutlineColor(sf::Color(0,0,0,200));
        barBg.setOutlineThickness(0.5f);
        target.draw(barBg);

        sf::RectangleShape barFill(sf::Vector2f(barW * norm, barH - 0.8f));
        barFill.setOrigin(0.f, barFill.getSize().y / 2.f);
        barFill.setPosition(p.x - barW/2.f + 0.5f, p.y - (float)m_tileSize*0.6f);
        sf::Uint8 r = (sf::Uint8)(255 * (1.f - norm));
        sf::Uint8 g = (sf::Uint8)(200 * norm + 55 * (1.f - norm));
        sf::Uint8 b = 40;
        barFill.setFillColor(sf::Color(r,g,b,220));
        target.draw(barFill);

        if (s_selectedId == e.id) {
            sf::CircleShape sel;
            if (e.species == Species::Herbivore) { sel = sf::CircleShape((float)m_tileSize * 0.6f, 30); }
            else sel = sf::CircleShape((float)m_tileSize * 0.5f, 30);
            sel.setOrigin(sel.getRadius(), sel.getRadius());
            sel.setPosition(p);
            sel.setFillColor(sf::Color::Transparent);
            sel.setOutlineColor(sf::Color(255,255,255,200));
            sel.setOutlineThickness(2.f);
            target.draw(sel);
        }

        s_lastGridPos[e.id] = e.pos;
    }

    // restore default view for UI drawing and input handling
    target.setView(defaultView);

    // mouse in window pixels (recompute to be safe)
    mousePixel = sf::Mouse::getPosition(target);

    // --- sidebar / HUD on right ---
    {
        sf::Vector2u ws = target.getSize();
        float sideW = 220.f;
        sf::RectangleShape sideBg(sf::Vector2f(sideW, (float)ws.y - 16.f));
        sideBg.setPosition((float)ws.x - sideW - 8.f, 8.f);
        sideBg.setFillColor(sf::Color(10,10,10,180));
        sideBg.setOutlineColor(sf::Color(255,255,255,40));
        sideBg.setOutlineThickness(1.f);
        target.draw(sideBg);

        if (s_uiFontLoaded) {
            sf::Text t;
            t.setFont(s_uiFont);
            t.setCharacterSize(14);
            t.setFillColor(sf::Color::White);

            // title
            t.setString("EcoSim - HUD");
            t.setPosition((float)ws.x - sideW + 6.f, 14.f);
            t.setCharacterSize(16);
            t.setStyle(sf::Text::Bold);
            target.draw(t);

            t.setCharacterSize(14);
            t.setStyle(sf::Text::Regular);
            // counts
            t.setString("Plants: " + std::to_string(countSpecies(Species::Plant)));
            t.setPosition((float)ws.x - sideW + 8.f, 46.f);
            target.draw(t);

            t.setString("Herbivores: " + std::to_string(countSpecies(Species::Herbivore)));
            t.setPosition((float)ws.x - sideW + 8.f, 66.f);
            target.draw(t);

            t.setString("Carnivores: " + std::to_string(countSpecies(Species::Carnivore)));
            t.setPosition((float)ws.x - sideW + 8.f, 86.f);
            target.draw(t);

            // selected entity info
            t.setString("Selected:");
            t.setPosition((float)ws.x - sideW + 8.f, 120.f);
            t.setCharacterSize(13);
            target.draw(t);

            if (s_selectedId >= 0) {
                // find entity
                int idx = -1;
                for (int i=0;i<(int)m_entities.size();++i) if (m_entities[i].id == s_selectedId) { idx = i; break; }
                if (idx >= 0) {
                    const Entity &se = m_entities[idx];
                    sf::Text info;
                    info.setFont(s_uiFont);
                    info.setCharacterSize(13);
                    info.setFillColor(sf::Color(220,220,220));
                    info.setString(
                        (se.species==Species::Plant?"Plant\n": se.species==Species::Herbivore?"Herbivore\n":"Carnivore\n")
                        + std::string("ID: ") + std::to_string(se.id) + "\n"
                        + std::string("Pos: ") + std::to_string(se.pos.x) + "," + std::to_string(se.pos.y) + "\n"
                        + std::string("Energy: ") + std::to_string((int)se.energy) + "\n"
                        + std::string("Age: ") + std::to_string(se.age)
                    );
                    info.setPosition((float)ws.x - sideW + 8.f, 140.f);
                    target.draw(info);
                } else {
                    t.setString(" (gone)");
                    t.setPosition((float)ws.x - sideW + 8.f, 140.f);
                    target.draw(t);
                }
            } else {
                t.setString(" (none)");
                t.setPosition((float)ws.x - sideW + 8.f, 140.f);
                target.draw(t);
            }
        }
    }

    // tooltip for hovered entity (near mouse)
    if (hoveredEntityIdx >= 0 && s_uiFontLoaded) {
        const Entity &he = m_entities[hoveredEntityIdx];
        sf::Text tip;
        tip.setFont(s_uiFont);
        tip.setCharacterSize(12);
        tip.setFillColor(sf::Color::White);
        tip.setString(
            (he.species==Species::Plant?"Plant": he.species==Species::Herbivore?"Herbivore":"Carnivore")
            + std::string("  E:") + std::to_string((int)he.energy)
            + std::string("  A:") + std::to_string(he.age)
        );
        sf::FloatRect br = tip.getLocalBounds();
        tip.setPosition((float)mousePixel.x + 12.f, (float)mousePixel.y + 6.f);

        sf::RectangleShape bg(sf::Vector2f(br.width + 10.f, br.height + 8.f));
        bg.setPosition(tip.getPosition().x - 4.f, tip.getPosition().y - 4.f);
        bg.setFillColor(sf::Color(20,20,20,220));
        bg.setOutlineColor(sf::Color(255,255,255,50));
        bg.setOutlineThickness(1.f);
        target.draw(bg);
        target.draw(tip);
    }
}

void World::applyTool(Tool tool, const sf::Vector2i& gridPos) {
    if (gridPos.x < 0 || gridPos.y < 0 || gridPos.x >= m_width || gridPos.y >= m_height) return;
    int idx = gridPos.x + gridPos.y * m_width;

    switch (tool) {
        case Tool::AddWater:
            m_tiles[idx].type = TileType::Water;
            m_tiles[idx].moisture = 1.f;
            break;
        case Tool::RemoveWater:
            m_tiles[idx].type = TileType::Grass;
            m_tiles[idx].moisture = 0.4f;
            break;
        case Tool::SpawnPlant: {
            Entity e; e.id = m_nextId++; e.species = Species::Plant; e.pos = gridPos; e.energy = 1.f; e.age=0;
            m_entities.push_back(e);
            break;
        }
        case Tool::SpawnHerbivore: {
            Entity e; e.id = m_nextId++; e.species = Species::Herbivore; e.pos = gridPos; e.energy = 55.f; e.age=0;
            m_entities.push_back(e);
            break;
        }
        case Tool::SpawnCarnivore: {
            Entity e; e.id = m_nextId++; e.species = Species::Carnivore; e.pos = gridPos; e.energy = 100.f; e.age=0;
            m_entities.push_back(e);
            break;
        }
        case Tool::ForceMigrate: {
            // nudge nearby animals away from that point
            for (auto &ent : m_entities) {
                if (ent.species == Species::Herbivore || ent.species == Species::Carnivore) {
                    int dx = ent.pos.x - gridPos.x;
                    int dy = ent.pos.y - gridPos.y;
                    int d = std::abs(dx) + std::abs(dy);
                    if (d <= 8 && d>0) {
                        // push one step away
                        if (std::abs(dx) >= std::abs(dy)) ent.pos.x += (dx>0?1:-1);
                        else ent.pos.y += (dy>0?1:-1);
                        ent.pos.x = std::clamp(ent.pos.x, 0, m_width-1);
                        ent.pos.y = std::clamp(ent.pos.y, 0, m_height-1);
                    }
                }
            }
            break;
        }
        case Tool::EraseEntity: {
            int idxEntity = entityIndexAt(gridPos);
            if (idxEntity >= 0) m_entities.erase(m_entities.begin() + idxEntity);
            break;
        }
        default: break;
    }
}

int World::entityIndexAt(const sf::Vector2i& p) const {
    for (int i = 0; i < (int)m_entities.size(); ++i) {
        if (m_entities[i].pos == p) return i;
    }
    return -1;
}

void World::rebuildVertexArray() {
    m_tileVerts.clear();
    m_tileVerts.setPrimitiveType(sf::Quads);
    m_tileVerts.resize(m_width * m_height * 4);

    for (int y=0;y<m_height;++y) for (int x=0;x<m_width;++x) {
        int i = (x + y * m_width) * 4;
        float px = (float)(x * m_tileSize);
        float py = (float)(y * m_tileSize);
        const Tile &t = m_tiles[x + y*m_width];
        sf::Color c = colorForTile(t);

        m_tileVerts[i+0].position = {px, py};
        m_tileVerts[i+1].position = {px + (float)m_tileSize, py};
        m_tileVerts[i+2].position = {px + (float)m_tileSize, py + (float)m_tileSize};
        m_tileVerts[i+3].position = {px, py + (float)m_tileSize};
        for (int k=0;k<4;++k) m_tileVerts[i+k].color = c;
    }
}
*/