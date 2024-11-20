#ifndef MATERIALSIMULATOR_H
#define MATERIALSIMULATOR_H

#include "GameState.h"

class MaterialSimulator {
    static void processTile(Grid& grid, int x, int y, Materials& materials);

public:
    static void process(Grid& grid, Materials& materials);
};



#endif //MATERIALSIMULATOR_H
