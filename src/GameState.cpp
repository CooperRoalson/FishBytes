#include "GameState.h"

#include "MaterialSimulator.h"
#include "BoidEntity.h"
#include "GameManager.h"

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

GameState::GameState(GameManager* gameManager, Vector2i size, double simSpeed)
        : gameManager(gameManager), grid(size), simSpeed(simSpeed) {}

void GameState::generateFrame(Ref<Image> image) {
    // Write material colors
    for (int x = 0; x < grid.size.x; ++x) {
        for (int y = 0; y < grid.size.y; ++y) {
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

Ref<JSON> GameState::exportData() {
    Dictionary data;

    data["config"] = configFile;

    Array size;
    size.append(grid.size.x);
    size.append(grid.size.y);
    data["size"] = size;

    Array gridData;
    gridData.resize(grid.size.x * grid.size.y);
    for (int y = 0; y < grid.size.y; ++y) {
        for (int x = 0; x < grid.size.x; ++x) {
            gridData[y * grid.size.x + x] = grid[x, y];
        }
    }
    data["grid"] = gridData;

    Array entityInstanceData;
    for (Entity* e : entityInstances) {
        Dictionary entityData;

        entityData["type"] = e->getType();

        Array position;
        position.append(e->getPosition().round().x);
        position.append(e->getPosition().round().y);
        entityData["position"] = position;

        entityInstanceData.append(entityData);
    }
    data["entityInstances"] = entityInstanceData;

    Ref<JSON> json = {memnew(JSON)};
    json->set_data(data);
    return json;
}

void GameState::importData(Ref<JSON> json) {
    Dictionary data = json->get_data();

    Array size = data.get_or_add("size", Array());
    if (size.size() != 2) { return; }
    clearGrid({size[0], size[1]});

    if (data.has("config")) {
        gameManager->importConfig(data["config"], false);
    };

    Array gridData = data.get_or_add("grid", Array());
    for (int y = 0; y < grid.size.y; ++y) {
        for (int x = 0; x < grid.size.x; ++x) {
            int i = y * grid.size.x + x;
            if (i >= gridData.size()) {
                break;
            }
            grid[x, y] = gridData[i];
        }
    }

    Array entityInstances = data.get_or_add("entityInstances", Array());
    for (int i = 0; i < entityInstances.size(); ++i) {
        Dictionary entityData = entityInstances[i];

        StringName type = entityData.get_or_add("type", "");

        Array position = entityData.get_or_add("position", Array());
        if (position.size() != 2) { continue; }
        spawnEntity({position[0], position[1]}, type);
    }
}

std::unique_ptr<GameState> GameState::clone() {
    std::unique_ptr<GameState> result = std::make_unique<GameState>(gameManager, grid.size, simSpeed);

    result->setConfig(configFile, materials, entities);
    for (int y = 0; y < grid.size.y; ++y) {
        for (int x = 0; x < grid.size.x; ++x) {
            result->grid[x, y] = grid[x, y];
        }
    }
    for (auto* e : entityInstances) {
        result->entityInstances.push_back(Entity::instantiateEntity(e->getType(), e->getProperties(), e->getPosition()));
    }

    return result;
}
