#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <bitset>

#include "Entities.h"
#include "godot_includes.h"
#include "Materials.h"

struct Pixel {
    StringName material;
};

struct Grid {
    std::vector<StringName> data;
    std::vector<bool> updated;
    int width, height;

    StringName& operator[](const int x, const int y) {
        if(x < 0 || x >= width && y < 0 && y >= height) {
            UtilityFunctions::printerr("Accessing invalid tile ", x, ", ", y);
            DEV_ASSERT(false);
        }
        return data[y * width + x];
    }

    const StringName& operator[](int x, int y) const {
        DEV_ASSERT(x >= 0 && x < width && y >= 0 && y < height);
        return data[y * width + x];
    }

    bool wasUpdated(const int x, const int y) {
        return updated[y * width + x] && (*this)[x,y] != StringName("");
    }

    void setUpdated(const int x, const int y) {
        updated[y * width + x] = true;
    }

    void finalizeUpdate() {
        for (auto && i : updated) {
            i = false;
        }
    }

    void swapTiles(const int x1, const int y1, const int x2, const int y2) {
        DEV_ASSERT(x1 >= 0 && x1 < width && y1 >= 0 && y1 < height);
        DEV_ASSERT(x2 >= 0 && x2 < width && y2 >= 0 && y2 < height);
        auto temp = (*this)[x1, y1];
        (*this)[x1, y1] = (*this)[x2, y2];
        (*this)[x2, y2] = temp;
        setUpdated(x1, y1);
        setUpdated(x2, y2);
    }

    Grid(const int width, const int height) : width(width), height(height), data(width * height), updated(width * height) {
        // data = new StringName[width * height];
    }
};

class GameState;

class Entity {
protected:
    StringName type;
    Ref<EntityProperties> properties;
    Vector2 position;
    bool dead = false;

    Entity(StringName type, Ref<EntityProperties> properties, Vector2 position) : type(type), properties(properties), position(position) {}

public:
    virtual ~Entity() = default;

    static Entity* instantiateEntity(StringName type, Ref<EntityProperties> properties, Vector2 position);

    virtual void render(Ref<Image> image);
    virtual void process(double delta, GameState& gameState) {}

    StringName getType() { return type; }
    Vector2 getPosition() { return position; }
    StringName getCurrentTile(GameState& gameState);
    bool isDead() { return dead; }
};

class GameState {
    Materials materials;
    Entities entities;

    Grid grid;
    std::vector<Entity*> entityInstances;

    double simSpeed;
    double timeSinceLastFrame = 0.0;

public:

    GameState(int width, int height, Materials materials, Entities entities, double simSpeed);

    ~GameState() {
        for (auto* e : entityInstances) {
            delete e;
        }
    }

    void setSimSpeed(double speed) {
        simSpeed = speed;
    }

    void generateFrame(Ref<Image> image);

    void process(double delta);

    // TODO: change to generator?
    // Could maybe be parallelized?
    void processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback);

    Ref<MaterialProperties> getMaterialProperties(StringName mat) {
        return materials.getProperties(mat);
    }

    Vector2i getDimensions() const {
        return {grid.width, grid.height};
    }

    bool isInBounds(Vector2i pos) const {
        return pos.x >= 0 && pos.x < grid.width && pos.y >= 0 && pos.y < grid.height;
    }

    StringName getTile(Vector2i const& pos) const {
        if (isInBounds(pos)) {
            return grid[pos.x, pos.y];
        }
        return "";
    }

    void setTile(Vector2i pos, StringName type) {
        if (isInBounds(pos)) {
            if (materials.getProperties(type).is_null()) {
                UtilityFunctions::printerr("Invalid mateiral type: ", type);
                return;
            }
            grid[pos.x, pos.y] = type;
        }
    }

    void spawnEntity(Vector2i pos, StringName type) {
        if (isInBounds(pos)) {
            auto properties = entities.getProperties(type);
            if (properties.is_null()) {
                UtilityFunctions::printerr("Invalid entity type: ", type);
                return;
            }
            entityInstances.push_back(Entity::instantiateEntity(type, properties, pos));
        }
    }

    Materials& getMaterials() { return materials; }
    Entities& getEntities() { return entities; }
};



#endif //GAMESTATE_H
