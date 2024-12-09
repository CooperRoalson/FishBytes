#include "GameState.h"

#include "MaterialSimulator.h"
#include "BoidEntity.h"

Entity* Entity::instantiateEntity(Ref<EntityProperties> properties, Vector2 position) {
    switch (properties->type) {
        case EntityProperties::STATIC:
            return new Entity(properties, position);
        case EntityProperties::BOID:
            return new BoidEntity(properties, position);
        default:
            DEV_ASSERT(false);
    }
}

void Entity::render(Ref<Image> image) {
    Vector2i pos = position.round();
    image->set_pixel(pos.x, pos.y, properties->color);
}

StringName Entity::getCurrentTile(GameState& gameState) {
    return gameState.getTile(position.round());
}

GameState::GameState(int width, int height, Materials materials, Entities entities, double simSpeed)
        : grid(width, height), materials(materials), entities(entities), simSpeed(simSpeed) {

    //v TODO: load default grid

    // // TESTING:
    // for (int x = 0; x < 25; ++x) {
    //     for (int y = 40; y < 50; y++) {
    //         grid[x, y] = "sand";
    //     }
    // }
    // grid[10, 49] = "food";
    // grid[15, 49] = "food";
    // grid[20, 49] = "food";
    //
    // for (int y = 10; y < 20; y++) {
    //     for (int x = 10; x < 50; ++x) {
    //         grid[x, y] = "water";
    //     }
    // }
    // grid[20, 21] = "water";
    //
    // Ref<BoidProperties> boidProps = entities.getProperties("boid");;
    // entityInstances.push_back(new BoidEntity(boidProps, Vector2(25, 45)));
    // entityInstances.push_back(new BoidEntity(boidProps, Vector2(25, 48)));
    // entityInstances.push_back(new BoidEntity(boidProps, Vector2(28, 45)));
    // entityInstances.push_back(new BoidEntity(boidProps, Vector2(28, 48)));
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
    for (auto& e : entityInstances) {
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
    for (auto& e : entityInstances) {
        e->process(delta, *this);
    }
}

void GameState::processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback) {
    // Could use spatial partition
    for (auto& e : entityInstances) {
        if ((e->getPosition() - position).length_squared() <= radius * radius) {
            callback(*e);
        }
    }
}