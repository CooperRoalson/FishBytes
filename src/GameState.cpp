#include "GameState.h"

#include "MaterialSimulator.h"
#include "BoidEntity.h"


GameState::GameState(int width, int height, Materials materials, double simSpeed)
        : grid(width, height), materials(materials), simSpeed(simSpeed) {

    // TODO: load default grid

    // TESTING:
    for (int x = 0; x < 50; ++x) {
        for (int y = 10; y < 30; y++) {
            grid[x, y] = "water";
        }
        for (int y = 40; y < 45; y++) {
            grid[x, y] = "sand";
        }
    }

    BoidSettings* settings = new BoidSettings{
        Color{"#ff0000", 1.0},
        10.0,
        5.0,
        3.0,
        1.0,
        1.0,
        1.0,
        1.0
    };

    entities.push_back(new BoidEntity(Vector2(25, 25), settings));
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

Ref<MaterialProperties> GameState::getMaterialProperties(Vector2i pos) {
    if (pos.x < 0 || pos.x >= grid.width || pos.y < 0 || pos.y >= grid.height) {
        return nullptr;
    }

    return materials.getProperties(grid[pos.x, pos.y]);
}
