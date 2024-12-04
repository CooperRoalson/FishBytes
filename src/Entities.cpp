#include "Entities.h"
#include "BoidEntity.h"

Entities Entities::defaultEntities() {
    Entities e;

    BoidProperties::BoidSettings boidSettings = {
        .groupRadius = 10.0,
        .tileRadius = 10,
        .maxSpeed = 50,
        .maxAccel = 50,
        .dragPercent = 0.2,
        .bouncePercent = 0.2,
        .separationWeight = 10.0,
        .alignmentPercent = 0.3,
        .cohesionWeight = 6.0,
        .obstacleWeight = 2.0,
        .tileWeights = Dictionary(),
    };

    boidSettings.tileWeights["food"] = 10.0;
    boidSettings.tileWeights.make_read_only();

    e.properties["boid"] = {memnew(BoidProperties(Color{"#FF0000", 1.0}, boidSettings))};

    return e;
}
