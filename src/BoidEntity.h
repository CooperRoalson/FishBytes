#ifndef BOIDENTITY_H
#define BOIDENTITY_H

#include "godot_includes.h"
#include "GameState.h"
#include "Entities.h"

struct BoidProperties : EntityProperties {
    struct BoidConfig : Resource {
        int visionRadius;
        double groupRadius;

        double maxSpeed;
        double maxAccel;

        double dragPercent;
        double bouncePercent;

        double separationWeight;
        double alignmentPercent;
        double cohesionWeight;

        double obstacleWeight;
        Dictionary tileWeights;
        Dictionary entityWeights;
        Array food;
        Array prey;
    };

    Ref<BoidConfig> boidConfig;
};

class BoidEntity : public Entity {
    Vector2 velocity;

public:
    BoidEntity(StringName type, Ref<EntityProperties> properties, Vector2 position);

    void process(double delta, GameState& gameState) override;

    bool hasLineOfSightTo(GameState& gameState, Vector2i pos);
};



#endif //BOIDENTITY_H
