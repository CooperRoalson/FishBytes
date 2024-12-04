#ifndef BOIDENTITY_H
#define BOIDENTITY_H

#include "godot_includes.h"
#include "GameState.h"
#include "Entities.h"

struct BoidProperties : EntityProperties {
    struct BoidSettings{
        double groupRadius;
        int tileRadius;

        double maxSpeed;
        double maxAccel;

        double dragPercent;
        double bouncePercent;

        double separationWeight;
        double alignmentPercent;
        double cohesionWeight;

        double obstacleWeight;
        Dictionary tileWeights;
    } boidSettings;

    BoidProperties() : EntityProperties(Color{"#000000", 1.0}, BOID) {}
    BoidProperties(Color color, BoidSettings settings) : EntityProperties(color, BOID), boidSettings(settings) {}
};

class BoidEntity : public Entity {
    Vector2 velocity;

public:
    BoidEntity(Ref<EntityProperties> properties, Vector2 position);

    void process(double delta, GameState& gameState) override;
};



#endif //BOIDENTITY_H
