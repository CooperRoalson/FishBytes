#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "godot_includes.h"
#include "Materials.h"

struct Grid {
    StringName* data;
    int width, height;

    StringName& operator[](int x, int y) {
        assert(x >= 0 && x < width && y >= 0 && y < height);
        return data[y * width + x];
    }

    Grid(int width, int height) : width(width), height(height) {
        data = new StringName[width * height];
    }

    ~Grid() {
        delete[] data;
    }
};

class GameState;

class Entity {
protected:
    Vector2 position;

public:
    explicit Entity(Vector2 position) : position(position) {}
    virtual ~Entity() = default;

    virtual void render(Ref<Image> image) = 0;
    virtual void process(double delta, GameState& gameState) = 0;

    Vector2 getPosition() { return position; }
};

class GameState {
    Materials materials;

    Grid grid;
    std::vector<Entity*> entities;

    double simSpeed;
    double timeSinceLastFrame = 0.0;

public:

    GameState(int width, int height, Materials materials, double simSpeed);

    void setSimSpeed(double speed) {
        simSpeed = speed;
    }

    void generateFrame(Ref<Image> image);

    void process(double delta);

    // TODO: change to generator?
    // Could maybe be parallelized?
    void processNearbyEntities(Vector2 position, double radius, const std::function<void(Entity&)>& callback);

    Ref<MaterialProperties> getMaterialProperties(Vector2i pos);

    Vector2i getDimensions() const { return {grid.width, grid.height}; }
};



#endif //GAMESTATE_H
