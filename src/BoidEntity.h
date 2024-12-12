#ifndef BOIDENTITY_H
#define BOIDENTITY_H

#include "godot_includes.h"
#include "GameState.h"
#include "Entities.h"

struct BoidProperties : EntityProperties {
    struct BoidConfig : Resource {
        int trailLen;

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

        static Ref<BoidConfig> parseBoidConfig(Dictionary& data);
    };

    Ref<BoidConfig> boidConfig;
};

class BoidEntity : public Entity {
    Vector2 velocity;

    std::deque<Vector2i> trail{};

public:
    BoidEntity(StringName type, Ref<EntityProperties> properties, Vector2 position);

    void process(double delta, GameState& gameState) override;

    void render(Ref<Image> image) override;

    bool hasLineOfSightTo(GameState& gameState, Vector2i pos);
};



#endif //BOIDENTITY_H
