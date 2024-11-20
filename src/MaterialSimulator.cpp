#include "MaterialSimulator.h"

#include "godot_includes.h"

void MaterialSimulator::process(Grid& grid, Materials& materials) {
    // Process tiles from bottom to top (and left to right)
    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            processTile(grid, x, y, materials);
        }
    }
}

void MaterialSimulator::processTile(Grid& grid, int x, int y, Materials& materials) {
    auto mat = grid[x, y];

    if (mat == String("")) {
        return;
    }

    auto properties = materials.getProperties(mat);

    switch (properties->type) {
        case MaterialProperties::EMPTY:
        case MaterialProperties::STATIC:
            break;
        case MaterialProperties::GRAVITY:
        case MaterialProperties::FLUID: {
            if (y > 0) {
                auto below = grid[x, y - 1];
                if (materials.isFluid(below)) {
                    grid[x, y - 1] = mat;
                    grid[x, y] = below;
                }
            }
            break;
        }
        default:
            UtilityFunctions::printerr("Unknown material type ", properties->type);
    }
}
