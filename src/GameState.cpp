#include "GameState.h"

#include "BehaviorEntity.h"
#include "MaterialSimulator.h"
#include "BoidEntity.h"
#include "GameManager.h"

Entity* Entity::instantiateEntity(const StringName &type, Ref<EntityProperties> properties, Vector2 position) {
    switch (properties->type) {
        case EntityProperties::STATIC:
            return new Entity(type, properties, position);
        case EntityProperties::BOID:
            return new BoidEntity(type, properties, position);
        case EntityProperties::BEHAVIOR:
            return new BehaviorEntity(type, properties, position);
    }
}

void Entity::render(Ref<Image> image) {
    Vector2i pos = position.round();
    image->set_pixel(pos.x, pos.y, properties->color);
}

Pixel Entity::getCurrentTile(const GameState& gameState) const {
    return gameState.getTile(position.round());
}

bool Entity::move(Vector2 vel, GameState& gameState, bool canGoInAir) {
    bool collided = false;
    position += vel;

    Vector2 bounded = position.clamp({0,0}, gameState.getDimensions() - Vector2i(1,1));
    if (bounded != position) {
        collided = true;
        position = bounded;
    }

    double amount = vel.length();
    Vector2 dir = vel.normalized();
    while (!Math::is_zero_approx(amount)
            && (canGoInAir ? gameState.getMaterialProperties(getCurrentTile(gameState))->isSolid()
                          : !gameState.getMaterialProperties(getCurrentTile(gameState))->isFluid())) {
        double sub = Math::min(amount, 1.0);
        position -= dir * sub;
        amount -= sub;
        collided = true;
    }
    position = position.clamp({0,0}, gameState.getDimensions() - Vector2i(1,1));
    return !collided;
}

bool Entity::hasLineOfSightTo(GameState& gameState, Vector2 pos) {
    Vector2 diff = pos - position;
    double dist = diff.length();
    Vector2 dir = diff / dist;
    for (int i = 0; i < dist; i++) {
        Vector2i checkPos = (position + dir * i).round();
        if (checkPos == pos) { break; }

        if (!gameState.isInBounds(checkPos)) { return false; }
        if (gameState.getMaterialProperties(gameState.getTile(checkPos))->isSolid()) { return false; }
    }
    return true;
}

GameState::GameState(GameManager* gameManager, Vector2i size, double tileSpeed, double entitySpeed)
        : gameManager(gameManager), grid(size), tileSpeed(tileSpeed), entitySpeed(entitySpeed) {}

void GameState::generateFrame(const Ref<Image>& image) {
    // Write material colors
    for (int x = 0; x < grid.size.x; ++x) {
        for (int y = 0; y < grid.size.y; ++y) {
            auto pixel = grid[x, y];
            const Ref<MaterialProperties> properties = materials.getProperties(pixel.material);
            Color color = properties->color;

            if (!properties->isFluid()) {
                if (pixel.colorOffset < 0) {
                    color = color.darkened(static_cast<float>(pixel.colorOffset * 0.03));
                } else {
                    color = color.lightened(static_cast<float>(pixel.colorOffset * 0.03));
                }
            }

            image->set_pixel(x, y, color);
        }
    }

    // Draw entities
    for (auto& e : entityInstances) {
        e->render(image);
    }
}

void GameState::process(double delta) {
    // Process tiles (based on simSpeed)
    double timePerFrame = 1.0 / tileSpeed;
    timeSinceLastFrame += delta;
    while (timeSinceLastFrame > timePerFrame) {
        MaterialSimulator::process(grid, materials);
        timeSinceLastFrame -= timePerFrame;
    }

    // Process entities
    delta *= entitySpeed;
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
    for (const auto& e : entityInstances) {
        if ((e->getPosition() - position).length_squared() <= radius * radius) {
            callback(*e);
        }
    }
}

Ref<JSON> GameState::exportData() {
    Dictionary data;

    data["config"] = configFile;
    data["size"] = UtilityFunctions::var_to_str(grid.size);

    Array gridData;
    gridData.resize(grid.size.x * grid.size.y);
    for (int y = 0; y < grid.size.y; ++y) {
        for (int x = 0; x < grid.size.x; ++x) {
            gridData[y * grid.size.x + x] = grid[x, y].material;
        }
    }
    data["grid"] = gridData;

    Array entityInstanceData;
    for (Entity* e : entityInstances) {
        Dictionary entityData;
        entityData["type"] = e->getType();
        entityData["position"] = UtilityFunctions::var_to_str(Vector2i(e->getPosition().round()));
        entityInstanceData.append(entityData);
    }
    data["entityInstances"] = entityInstanceData;

    Ref<JSON> json = {memnew(JSON)};
    json->set_data(data);
    return json;
}

void GameState::importData(Ref<JSON> json) {
    Dictionary data = json->get_data();

    Vector2i size = UtilityFunctions::str_to_var(data.get_or_add("size", "Vector2i(50, 50)"));
    clearGrid({size.x, size.y});

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
            grid[x, y] = Pixel{gridData[i]};
        }
    }

    Array entityInstances = data.get_or_add("entityInstances", Array());
    for (int i = 0; i < entityInstances.size(); ++i) {
        Dictionary entityData = entityInstances[i];

        StringName type = entityData.get_or_add("type", "");

        Vector2i position = UtilityFunctions::str_to_var(entityData.get_or_add("position", "Vector2i(0, 0)"));
        spawnEntity({position.x, position.y}, type);
    }
}

std::unique_ptr<GameState> GameState::clone() {
    std::unique_ptr<GameState> result = std::make_unique<GameState>(gameManager, grid.size, tileSpeed, entitySpeed);

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
