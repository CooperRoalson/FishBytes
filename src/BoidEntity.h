#ifndef BOIDENTITY_H
#define BOIDENTITY_H

#include "godot_includes.h"
#include "GameState.h"

struct BoidSettings {
    Color color;

    double groupRadius;
    double maxSpeed;
    double maxAccel;

    double separationWeight;
    double alignmentWeight;
    double cohesionWeight;
    double obstacleWeight;
};

class BoidEntity : public Entity {
    Vector2 velocity;

    BoidSettings* settings;

public:
    BoidEntity(Vector2 position, BoidSettings* settings);

    void render(Ref<Image> image) override;

    void process(double delta, GameState& gameState) override;
};



#endif //BOIDENTITY_H
