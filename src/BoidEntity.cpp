#include "BoidEntity.h"

BoidEntity::BoidEntity(Ref<EntityProperties> properties, Vector2 position) : Entity(properties, position) {
    // TODO: make this more configurable
    velocity = Vector2::from_angle(UtilityFunctions::randf_range(0, Math_TAU)) * 10;

    // Make sure the properties is a BoidProperties
    (void) Object::cast_to<BoidProperties>(properties.ptr());
}

void BoidEntity::process(double delta, GameState& gameState) {
    Vector2 acceleration = Vector2();

    BoidProperties::BoidSettings& settings = Object::cast_to<BoidProperties>(properties.ptr())->boidSettings;

    // Boid-related forces
    {
        Vector2 groupPos = Vector2();
        Vector2 groupVel = Vector2();
        int groupSize = 0;

        gameState.processNearbyEntities(position, settings.groupRadius, [&] (Entity& e) {
            BoidEntity* boid = dynamic_cast<BoidEntity*>(&e);
            if (!boid) { return; }

            groupPos += boid->position;
            groupVel += boid->velocity;
            groupSize++;

            Vector2 diff = boid->position - position;
            double distSquared = diff.length_squared();
            if (Math::is_zero_approx(distSquared)) { return; }

            // Separation
            acceleration += -diff * (1 / distSquared) * settings.separationWeight; // Proportional to 1/distance
        });

        groupPos /= groupSize;
        groupVel /= groupSize;

        Vector2 diff = groupPos - position;
        double distSquared = diff.length_squared();
        double dist = Math::sqrt(distSquared);

        if (!Math::is_zero_approx(distSquared)) {
            // TODO: make attenuation controllable?

            // Cohesion
            acceleration += diff * settings.cohesionWeight; // Proportional to distance

            // Alignment
            acceleration += (groupVel - velocity) * settings.alignmentPercent; // Proportional to 1

            // Drag
            acceleration += -velocity * (1 - Math::pow(1 - settings.dragPercent, delta));
        }
    }

    // Tile-related forces
    const Vector2i posI = position.round();
    for (int x = -settings.tileRadius; x <= settings.tileRadius; ++x) {
        for (int y = -settings.tileRadius; y <= settings.tileRadius; ++y) {
            Vector2i pos = posI + Vector2i(x, y);
            StringName mat = gameState.getTile(pos);
            Ref<MaterialProperties> properties = gameState.getMaterialProperties(mat);

            Vector2 diff = pos - position;
            double distSquared = diff.length_squared();
            double dist = Math::sqrt(distSquared);
            if (Math::is_zero_approx(distSquared)) { continue; }

            double weight = 0;
            if (settings.tileWeights.has(mat)) {
                weight = double(settings.tileWeights[mat]); // Proportional to 1
            } else if (!gameState.isInBounds(pos) || properties->isSolid()) {
                weight = -settings.obstacleWeight * (1 / distSquared ); // Proportional to 1/distance^2
            }

            acceleration += diff * (1 / dist) * weight;
        }
    }

    acceleration = acceleration.limit_length(settings.maxAccel);

    velocity += acceleration * delta;
    velocity = velocity.limit_length(settings.maxSpeed);

    position += velocity * delta;
    position = position.clamp({0,0}, gameState.getDimensions() - Vector2i(1,1));

    // TODO: make this configurable
    if (getCurrentTile(gameState) == StringName("food")) {
        gameState.setTile(position.round(), "");
    }

    // TODO: prevent running into obstacles or out of water
    double amount = velocity.length() * delta;
    Vector2 dir = velocity.normalized();
    while (!Math::is_zero_approx(amount) && gameState.getMaterialProperties(getCurrentTile(gameState))->isSolid()) {
        double sub = Math::min(amount, 1.0);
        position -= dir * sub;
        velocity *= -settings.bouncePercent;
        amount -= sub;
    }
}
