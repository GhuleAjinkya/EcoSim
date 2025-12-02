class Constants {
    public:
    // Tile & World
    static constexpr int GRID_SEED = 70;
    static constexpr int TILE_DECAY_COOLDOWN = 12;
    
    // Entity Health & Aging
    static constexpr int HERBIVORE_MAX_HEALTH = 100;
    static constexpr int CARNIVORE_MAX_HEALTH = 120;
    static constexpr int INITIAL_ENTITY_HEALTH = 100;
    
    // Herbivore Behavior
    static constexpr int HERBIVORE_SEARCH_RADIUS = 5;
    static constexpr int HERBIVORE_HUNT_HEALTH_THRESHOLD = 75;
    static constexpr int HERBIVORE_FEED_HEALTH_THRESHOLD = 50;
    static constexpr int HERBIVORE_FEED_GAIN = 80;
    static constexpr int HERBIVORE_MOVEMENT_DELAY = 30;
    static constexpr int HERBIVORE_WANDER_CHANCE = 50;  // rand() % 2 == 0 = 50%
    static constexpr int HERBIVORE_REPRODUCTION_AGE = 500;
    static constexpr int HERBIVORE_REPRODUCTION_HEALTH = 75;
    static constexpr int HERBIVORE_REPRODUCTION_CHANCE = 2;  // per 1000 ticks
    static constexpr int HERBIVORE_MAX_NEARBY_TO_REPRODUCE = 4;
    static constexpr int HERBIVORE_CHILD_HEALTH_PCT = 35;  // 35% of parent health
    
    // Carnivore Behavior
    static constexpr int CARNIVORE_SEARCH_RADIUS = 7;
    static constexpr int CARNIVORE_HUNT_HEALTH_THRESHOLD = 60;
    static constexpr int CARNIVORE_ATTACK_HEALTH_THRESHOLD = 40;
    static constexpr int CARNIVORE_ATTACK_DAMAGE = 150;
    static constexpr int CARNIVORE_ATTACK_GAIN = 75;
    static constexpr int CARNIVORE_MOVEMENT_DELAY = 30;
    static constexpr int CARNIVORE_WANDER_CHANCE = 50;  // rand() % 2 == 0 = 50%
    static constexpr int CARNIVORE_MATE_HEALTH = 70;
    static constexpr int CARNIVORE_MATE_AGE = 750;
    static constexpr int CARNIVORE_MATE_COOLDOWN = 750;
    static constexpr int CARNIVORE_MATE_RANGE = 3;  // search radius for mate
    static constexpr int CARNIVORE_MATE_PROXIMITY = 3;  // spawn distance when mating
    static constexpr int CARNIVORE_MATE_HEALTH_COST = 40;  // health lost per parent when reproducing
    static constexpr int CARNIVORE_MAX_NEARBY_TO_SEARCH_MATE = 999999;  // no limit (find nearest)
    
    // Health Decay
    static constexpr int HERBIVORE_HEALTH_DECAY = 1;
    static constexpr int HERBIVORE_DECAY_CHANCE = 5;  // rand() % 5 == 0
    static constexpr int CARNIVORE_HEALTH_DECAY = 1;
    static constexpr int CARNIVORE_DECAY_CHANCE = 5;  // rand() % 5 == 0
    
    // Spawning
    static constexpr int HERBIVORE_SPAWN_MIN_GROUP_SIZE = 2;
    static constexpr int HERBIVORE_SPAWN_MAX_GROUP_SIZE = 5;
    static constexpr int HERBIVORE_SPAWN_CLUSTER_RADIUS = 2;  // ±2 tiles around center
    static constexpr int HERBIVORE_MAX_TOTAL = 30;
    static constexpr int CARNIVORE_MAX_TOTAL = 5;
    static constexpr int SPAWN_MAX_ATTEMPTS = 2000;
    static constexpr int SPAWN_INNER_ATTEMPTS = 200;
};
/*
Suggested UI features to add (prioritized)

Core HUD

species counts (Plants / Herbivores / Carnivores) in top of sidebar (like worldold).
play/pause button and simulation speed control (1x, 2x, 0.5x or slider).
step/frame button for debugging single-step updates.
Interaction Tools


persistent Selected Entity panel that follows the entity by ID (already present) — add a “Follow” toggle to pan/zoom to tracked entity.
a "Find by ID" input and "Center on entity" button.
Entity & Tile Visuals

on-hover tooltip showing species, health/energy, age (like worldold).
draw health/energy bars above entities (normalized by species cap).
orientation indicator (small rotation) and subtle bobbing/shadow to improve readability.
Camera & View

pan (arrow keys / mouse drag) and zoom in/out, with world coordinates shown in HUD.
mini-map overview (optional).
Diagnostics & Graphs

time-series plot (counts vs time) for species populations.
toggleable debug overlay: show occupied tile IDs, movementDelay values, RNG seed, FPS, entity count.
performance stats (draw/update timings).
Controls & Persistence

random seed input + "regenerate" button for repeatability.
save/load world state (serialize tiles + entities).
Reset world button and adjustable initial spawn counts.
UX polish

use an in-project font and render sidebar text nicely (not just DrawText).
disable spawn when clicked over water/rock, show feedback.
confirmation dialogs for destructive actions (reset, erase many).
Implementation hints mapped to existing code

For plant-as-entity behavior, either:
convert Tile-based grass to Entity::Plant (port worldold entity energy logic), or
emulate plant energy/regen per tile but adopt dt-scaled growth and consumption rates from worldold.
Add dt (frame time) to update pipeline to make reproduction/chance deterministic and frame-rate independent.
Implement vertex-batch drawing for tiles (optional) to mimic worldold performance.
Use const references when iterating entities in draw/UI to avoid copies (you already have some copies to fix).
Keep occupancy updates consistent: when an entity dies, ensure its tile.freeOccupant() runs.
If you want, next steps I can implement (pick one)

Add tools palette and mouse/keyboard handling for spawning/editing tiles.
Convert Tile grass to Plant entities (port stepPlants from worldold).
Add interpolation smoothing for entity rendering (prevPos + interp).
Implement sidebar species counts + mini charts.
Which of these should I implement next?
*/