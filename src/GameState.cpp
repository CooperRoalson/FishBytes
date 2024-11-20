#include "GameState.h"

#include "MaterialSimulator.h"


GameState::GameState(int width, int height, Materials materials, double simSpeed)
        : grid(width, height), materials(materials), simSpeed(simSpeed) {

    // TODO: load default grid

    grid[5, 2] = "water";
    grid[5, 40] = "sand";
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
    // TODO
}
