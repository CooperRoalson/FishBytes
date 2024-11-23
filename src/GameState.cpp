#include "GameState.h"

#include "MaterialSimulator.h"
#include "BoidEntity.h"


StringName Entity::getCurrentTile(GameState& gameState) {
    return gameState.getTile(position.round());
}

GameState::GameState(int width, int height, Materials materials, double simSpeed)
        : grid(width, height), materials(materials), simSpeed(simSpeed) {

    // TODO: load default grid

    // TESTING:
    for (int x = 0; x < 25; ++x) {
        for (int y = 40; y < 50; y++) {
            grid[x, y] = "sand";
        }
    }
    grid[10, 49] = "food";
    grid[15, 49] = "food";
    grid[20, 49] = "food";

    for (int y = 10; y < 20; y++) {
        for (int x = 10; x < 50; ++x) {
            grid[x, y] = "water";
        }
    }
    grid[20, 21] = "water";

    BoidSettings* settings = new BoidSettings{
        .color = Color{"#ff0000", 1.0},
        .groupRadius = 10.0,
        .tileRadius = 10,
        .maxSpeed = 50,
        .maxAccel = 50,
        .dragPercent = 0.2,
        .bouncePercent = 0.2,
        .separationWeight = 10.0,
        .alignmentPercent = 0.3,
        .cohesionWeight = 6.0,
        .obstacleWeight = 2.0,
        .tileWeights = Dictionary(),
    };

    settings->tileWeights["food"] = 10.0;
    settings->tileWeights.make_read_only();

    entities.push_back(new BoidEntity(Vector2(25, 45), settings));
    entities.push_back(new BoidEntity(Vector2(25, 48), settings));
    entities.push_back(new BoidEntity(Vector2(28, 45), settings));
    entities.push_back(new BoidEntity(Vector2(28, 48), settings));
}

void GameState::generateFrame(Ref<Image> image) {
    // Write material colors
    for (int x = 0; x < grid.width; ++x) {
        for (int y = 0; y < grid.height; ++y) {
            auto mat = grid[x, y];
            Ref<MaterialProperties> properties = materials.getProperties(mat);
            Color c = properties.is_null() ? Color() : properties->color;
            image->set_pixel(x, y, c);
        }
    }

    // Draw entities
    for (auto& e : entities) {
        e->render(image);
    }
}

void GameState::process(double delta) {
    // Process tiles (based on simSpeed)
    double timePerFrame = 1.0 / simSpeed;
    timeSinceLastFrame += delta;
    while (timeSinceLastFrame > timePerFrame) {
        MaterialSimulator::process(grid, materials);
        timeSinceLastFrame -= timePerFrame;
    }

    // Process entities
    for (auto& e : entities) {
        e->process(delta, *this);
    }
}

void GameState::processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback) {
    // Could use spatial partition
    for (auto& e : entities) {
        if ((e->getPosition() - position).length_squared() <= radius * radius) {
            callback(*e);
        }
    }
}