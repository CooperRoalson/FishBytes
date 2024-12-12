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
    auto mat = grid[x, y].material;

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
                auto below_mat = grid[x, y - 1].material;
                if (!materials.getProperties(below_mat)->isSolid() && !grid.wasUpdated(x, y - 1)) {
                    grid.swapTiles(x, y, x, y - 1);
                } else {
                    // try to flow bottom left or bottom right
                    if (x > 0 && !materials.getProperties(grid[x - 1, y - 1].material)->isSolid() && !grid.wasUpdated(x - 1, y - 1)) {
                        grid.swapTiles(x, y, x - 1, y - 1);
                        break;
                    }
                    if (x < grid.size.x - 1 && !materials.getProperties(grid[x + 1, y - 1].material)->isSolid() && !grid.wasUpdated(x + 1, y - 1)) {
                        grid.swapTiles(x, y, x + 1, y - 1);
                        break;
                    }
                }
            }

            break;
        case MaterialProperties::FLUID: {
            // check directly below first
            if (y > 0) {
                auto below_mat = grid[x, y - 1].material;
                if (materials.getProperties(below_mat)->type == MaterialProperties::EMPTY) {
                    grid.swapTiles(x, y, x, y - 1);
                    break;
                }

                // if can't move straight down, try diagonal down
                if (x > 0) {
                    auto belowLeftMat = grid[x - 1, y - 1].material;
                    if (!grid.wasUpdated(x - 1, y - 1) &&
                        materials.getProperties(belowLeftMat)->type == MaterialProperties::EMPTY) {
                        grid.swapTiles(x, y, x - 1, y - 1);
                        break;
                    }
                }
                if (x < grid.size.x - 1) {
                    auto belowRightMaterial = grid[x + 1, y - 1].material;
                    if (!grid.wasUpdated(x + 1, y - 1) &&
                        materials.getProperties(belowRightMaterial)->type == MaterialProperties::EMPTY) {
                        grid.swapTiles(x, y, x + 1, y - 1);
                        break;
                    }
                }
            }

            // if can't move down at all, spread horizontally
            bool canFlowLeft = x > 0 && !grid.wasUpdated(x - 1, y) &&
                               materials.getProperties(grid[x - 1, y].material)->type == MaterialProperties::EMPTY;
            bool canFlowRight = x < grid.size.x - 1 && !grid.wasUpdated(x + 1, y) &&
                                materials.getProperties(grid[x + 1, y].material)->type == MaterialProperties::EMPTY;

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
