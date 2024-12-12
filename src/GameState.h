#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <bitset>
#include <utility>

#include "Entities.h"
#include "godot_includes.h"
#include "Materials.h"

// Forward declaration
class GameManager;

struct Pixel {
    StringName material{""};
    char colorOffset{0};

    explicit Pixel(StringName material)
        : material(std::move(material)) {
        // set to random color offset between -3 and 3
        colorOffset = static_cast<char>(UtilityFunctions::randi_range(-3, 3));
    }

    explicit Pixel() = default;
};

struct Grid {
    std::vector<Pixel> data;
    std::vector<bool> updated;
    Vector2i size;

    Pixel& operator[](const int x, const int y) {
        if(x < 0 || x >= size.x && y < 0 && y >= size.y) {
            UtilityFunctions::printerr("Accessing invalid tile ", x, ", ", y);
            DEV_ASSERT(false);
        }
        return data[y * size.x + x];
    }

    const Pixel& operator[](int x, int y) const {
        DEV_ASSERT(x >= 0 && x < size.x && y >= 0 && y < size.y);
        return data[y * size.x + x];
    }

    bool wasUpdated(const int x, const int y) {
        return updated[y * size.x + x] && ((*this)[x,y]).material != StringName("");
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

    explicit Grid(Vector2i size) : data(size.x * size.y), updated(size.x * size.y), size(size) {}
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

    static Entity* instantiateEntity(const StringName &type, Ref<EntityProperties> properties, Vector2 position);

    virtual void render(Ref<Image> image);
    virtual void process(double delta, GameState& gameState) {}

    StringName getType() { return type; }
    Vector2 getPosition() const { return position; }
    Ref<EntityProperties> getProperties() { return properties; }
    Pixel getCurrentTile(const GameState& gameState) const;
    bool isDead() { return dead; }
    void die() { dead = true;}

    bool move(Vector2 vel, GameState& gameState, bool canGoInAir = false);
    bool hasLineOfSightTo(GameState& gameState, Vector2 pos);
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

    void generateFrame(const Ref<Image>& image);

    void process(double delta);

    // TODO: change to generator?
    // Could maybe be parallelized?
    void processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback);

    Ref<MaterialProperties> getMaterialProperties(StringName mat) {
        auto result = materials.getProperties(mat);
        DEV_ASSERT(result.is_valid());
        return result;
    }

    Ref<MaterialProperties> getMaterialProperties(const Pixel& p) {
        return materials.getProperties(p.material);
    }

    Vector2i getDimensions() const { return grid.size; }

    [[nodiscard]] bool isInBounds(const Vector2i pos) const {
        return pos.x >= 0 && pos.x < grid.size.x && pos.y >= 0 && pos.y < grid.size.y;
    }

    [[nodiscard]] Pixel getTile(Vector2i const& pos) const {
        if (isInBounds(pos)) {
            return grid[pos.x, pos.y];
        }
        UtilityFunctions::printerr("Accessing invalid tile ", pos.x, ", ", pos.y);
        return Pixel{""};
    }

    void setTile(Vector2i pos, const Pixel& p) {
        StringName type = p.material;
        if (isInBounds(pos)) {
            if (materials.getProperties(type).is_null()) {
                UtilityFunctions::printerr("Invalid mateiral type: ", type);
                return;
            }
            grid[pos.x, pos.y] = p;
        }
    }

    void spawnEntity(const Vector2i pos, const StringName& type) {
        if (isInBounds(pos)) {
            const auto properties = entities.getProperties(type);
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
