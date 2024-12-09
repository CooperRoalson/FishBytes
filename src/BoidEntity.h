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
    };

    Ref<BoidConfig> boidConfig;
};

class BoidEntity : public Entity {
    Vector2 velocity;

public:
    BoidEntity(StringName type, Ref<EntityProperties> properties, Vector2 position);

    void process(double delta, GameState& gameState) override;
};



#endif //BOIDENTITY_H
