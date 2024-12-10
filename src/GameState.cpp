#include "GameState.h"

#include "MaterialSimulator.h"
#include "BoidEntity.h"

Entity* Entity::instantiateEntity(StringName type, Ref<EntityProperties> properties, Vector2 position) {
    switch (properties->type) {
        case EntityProperties::STATIC:
            return new Entity(type, properties, position);
        case EntityProperties::BOID:
            return new BoidEntity(type, properties, position);
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

GameState::GameState(int width, int height, double simSpeed)
        : grid(width, height), simSpeed(simSpeed) {}

void GameState::generateFrame(Ref<Image> image) {
    // Write material colors
    for (int x = 0; x < grid.width; ++x) {
        for (int y = 0; y < grid.height; ++y) {
            auto mat = grid[x, y];
            Ref<MaterialProperties> properties = materials.getProperties(mat);
            image->set_pixel(x, y, properties->color);
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
    for (int i = 0; i < entityInstances.size(); ++i) {
        entityInstances[i]->process(delta, *this);
        if (entityInstances[i]->isDead()) {
            delete entityInstances[i];
            entityInstances.erase(entityInstances.begin() + i);
            --i;
        }
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