#include "MaterialSimulator.h"

#include "godot_includes.h"

void MaterialSimulator::process(Grid &grid, Materials &materials) {
    // Process tiles from bottom to top (and left to right)
    for (int y = 0; y < grid.size.y; ++y) {
        for (int x = 0; x < grid.size.x; ++x) {
            processTile(grid, x, y, materials);
        }
    }

    grid.finalizeUpdate();
}

void MaterialSimulator::processTile(Grid &grid, int x, int y, Materials &materials) {
    auto mat = grid[x, y];

    if (mat.is_empty()) {
        return;
    }


    auto properties = materials.getProperties(mat);

    switch (properties->type) {
        case MaterialProperties::EMPTY:
        case MaterialProperties::STATIC:
            break;
        case MaterialProperties::GRAVITY:
            // basic falling sand game physics
            if (y > 0) {
                auto below = grid[x, y - 1];
                if (!materials.getProperties(below)->isSolid() && !grid.wasUpdated(x, y - 1)) {
                    grid.swapTiles(x, y, x, y - 1);
                } else {
                    // try to flow bottom left or bottom right
                    if (x > 0 && !materials.getProperties(grid[x - 1, y - 1])->isSolid() && !grid.wasUpdated(x - 1, y - 1)) {
                        grid.swapTiles(x, y, x - 1, y - 1);
                        break;
                    }
                    if (x < grid.size.x - 1 && !materials.getProperties(grid[x + 1, y - 1])->isSolid() && !grid.wasUpdated(x + 1, y - 1)) {
                        grid.swapTiles(x, y, x + 1, y - 1);
                        break;
                    }
                }
            }

            break;
        case MaterialProperties::FLUID: {
            // check directly below first
            if (y > 0) {
                auto below = grid[x, y - 1];
                if (materials.getProperties(below)->type == MaterialProperties::EMPTY) {
                    grid.swapTiles(x, y, x, y - 1);
                    break;
                }

                // if can't move straight down, try diagonal down
                if (x > 0) {
                    auto belowLeft = grid[x - 1, y - 1];
                    if (!grid.wasUpdated(x - 1, y - 1) &&
                        materials.getProperties(belowLeft)->type == MaterialProperties::EMPTY) {
                        grid.swapTiles(x, y, x - 1, y - 1);
                        break;
                    }
                }
                if (x < grid.size.x - 1) {
                    auto belowRight = grid[x + 1, y - 1];
                    if (!grid.wasUpdated(x + 1, y - 1) &&
                        materials.getProperties(belowRight)->type == MaterialProperties::EMPTY) {
                        grid.swapTiles(x, y, x + 1, y - 1);
                        break;
                    }
                }
            }

            // if can't move down at all, spread horizontally
            bool canFlowLeft = x > 0 && !grid.wasUpdated(x - 1, y) &&
                               materials.getProperties(grid[x - 1, y])->type == MaterialProperties::EMPTY;
            bool canFlowRight = x < grid.size.x - 1 && !grid.wasUpdated(x + 1, y) &&
                                materials.getProperties(grid[x + 1, y])->type == MaterialProperties::EMPTY;

            if (canFlowLeft && canFlowRight) {
                // randomly choose direction if both are available
                if (UtilityFunctions::randi() % 2 == 0) {
                    grid.swapTiles(x, y, x - 1, y);
                } else {
                    grid.swapTiles(x, y, x + 1, y);
                }
            } else if (canFlowLeft) {
                grid.swapTiles(x, y, x - 1, y);
            } else if (canFlowRight) {
                grid.swapTiles(x, y, x + 1, y);
            }
            break;
        }
        default:
            UtilityFunctions::printerr("Unknown material type ", properties->type);
    }
}
