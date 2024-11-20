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

class GameState {
    Materials materials;

    Grid grid;

    double simSpeed;
    double timeSinceLastFrame = 0.0;

public:

    GameState(int width, int height, Materials materials, double simSpeed);

    void setSimSpeed(double speed) {
        simSpeed = speed;
    }

    void generateFrame(Ref<Image> image);

    void process(double delta);
};



#endif //GAMESTATE_H
