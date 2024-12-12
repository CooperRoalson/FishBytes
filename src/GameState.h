#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <bitset>

#include "Entities.h"
#include "godot_includes.h"
#include "Materials.h"

// Forward declaration
class GameManager;

struct Pixel {
    StringName material;
};

struct Grid {
    std::vector<StringName> data;
    std::vector<bool> updated;
    Vector2i size;

    StringName& operator[](const int x, const int y) {
        if(x < 0 || x >= size.x && y < 0 && y >= size.y) {
            UtilityFunctions::printerr("Accessing invalid tile ", x, ", ", y);
            DEV_ASSERT(false);
        }
        return data[y * size.x + x];
    }

    const StringName& operator[](int x, int y) const {
        DEV_ASSERT(x >= 0 && x < size.x && y >= 0 && y < size.y);
        return data[y * size.x + x];
    }

    bool wasUpdated(const int x, const int y) {
        return updated[y * size.x + x] && (*this)[x,y] != StringName("");
    }

    void setUpdated(const int x, const int y) {
        updated[y * size.x + x] = true;
    }

    void finalizeUpdate() {
        for (auto && i : updated) {
            i = false;
        }
    }

    void swapTiles(const int x1, const int y1, const int x2, const int y2) {
        DEV_ASSERT(x1 >= 0 && x1 < size.x && y1 >= 0 && y1 < size.y);
        DEV_ASSERT(x2 >= 0 && x2 < size.x && y2 >= 0 && y2 < size.y);
        auto temp = (*this)[x1, y1];
        (*this)[x1, y1] = (*this)[x2, y2];
        (*this)[x2, y2] = temp;
        setUpdated(x1, y1);
        setUpdated(x2, y2);
    }

    void reset(Vector2i sz) {
        size = sz;

        data.clear();
        data.resize(size.x * size.y);

        updated.clear();
        updated.resize(size.x * size.y);
    }

    Grid(Vector2i size) : size(size), data(size.x * size.y), updated(size.x * size.y) {}
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
    Ref<EntityProperties> getProperties() { return properties; }
    StringName getCurrentTile(GameState& gameState);
    bool isDead() { return dead; }
    void die() { dead = true;}
    bool move(Vector2 vel, GameState& gameState, bool canGoInAir = false);
};

class GameState {
    GameManager* gameManager;

    String configFile;
    Materials materials;
    Entities entities;

    Grid grid;
    std::vector<Entity*> entityInstances;

    double tileSpeed, entitySpeed;
    double timeSinceLastFrame = 0.0;

public:

    GameState(GameManager* gameManager, Vector2i size, double tileSpeed, double entitySpeed);

    ~GameState() {
        for (auto* e : entityInstances) {
            delete e;
        }
    }

    void setConfig(String configFile, Materials materials, Entities entities) {
        this->configFile = configFile;
        this->materials = materials;
        this->entities = entities;
    }

    void setSimSpeed(double tileSpeed, double entitySpeed) {
        this->tileSpeed = tileSpeed;
        this->entitySpeed = entitySpeed;
    }

    void generateFrame(Ref<Image> image);

    void process(double delta);

    // TODO: change to generator?
    // Could maybe be parallelized?
    void processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback);

    Ref<MaterialProperties> getMaterialProperties(StringName mat) {
        return materials.getProperties(mat);
    }

    Vector2i getDimensions() const { return grid.size; }

    bool isInBounds(Vector2i pos) const {
        return pos.x >= 0 && pos.x < grid.size.x && pos.y >= 0 && pos.y < grid.size.y;
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

    void clearGrid(Vector2i size = {-1, -1}) {
        if (size == Vector2i(-1, -1)) {
            size = grid.size;
        }
        grid.reset(size);
        for (auto* e : entityInstances) {
            delete e;
        }
        entityInstances.clear();
    }

    Materials& getMaterials() { return materials; }
    Entities& getEntities() { return entities; }

    Ref<JSON> exportData();
    void importData(Ref<JSON> data);

    std::unique_ptr<GameState> clone();
};



#endif //GAMESTATE_H
