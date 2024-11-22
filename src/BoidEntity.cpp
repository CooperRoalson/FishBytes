#include "BoidEntity.h"

BoidEntity::BoidEntity(Vector2 position, BoidSettings* settings) : Entity(position), settings(settings) {
    velocity = Vector2(UtilityFunctions::randf_range(-2, 2),
                       UtilityFunctions::randf_range(-2, 2));
}

void BoidEntity::render(Ref<Image> image) {
    Vector2i pos = position.round(); // convert to int position
    image->set_pixel(pos.x, pos.y, settings->color);
}

void BoidEntity::process(double delta, GameState& gameState) {
    Vector2 acceleration = Vector2();

    gameState.processNearbyEntities(position, settings->groupRadius, [&] (Entity& e) {
        if (&e == this) { return; }

        BoidEntity* boid = dynamic_cast<BoidEntity*>(&e);
        if (!boid) { return; }

        Vector2 diff = boid->position - position;
        double distSquared = diff.length_squared();

        if (Math::is_zero_approx(distSquared)) { return; }

        // TODO: make attenuation controllable?

        // Separation
        acceleration += -diff * (1 / distSquared) * settings->separationWeight; // essentially proportional to 1/distance

        // Alignment
        acceleration += boid->velocity * settings->alignmentWeight;

        // Cohesion
        acceleration += diff * settings->cohesionWeight; // essentially proportional to distance
    });

    // Obstacle avoidance
    // TODO: make the 3 tunable
    for (int x = -3; x <= 3; ++x) {
        for (int y = -3; y <= 3; ++y) {
            Vector2i pos = Vector2i(position.round()) + Vector2i(x, y);
            Ref<MaterialProperties> properties = gameState.getMaterialProperties(pos);

            if (properties.is_null() || properties->isSolid()) {
                Vector2 diff = pos - position;
                double distSquared = diff.length_squared();
                if (Math::is_zero_approx(distSquared)) { return; }

                acceleration += -diff * (1 / distSquared) * settings->obstacleWeight;
            }
        }
    }

    acceleration = acceleration.limit_length(settings->maxAccel);

    velocity += acceleration * delta;
    velocity = velocity.limit_length(settings->maxSpeed);

    position += velocity * delta;
    position = position.clamp({0,0}, gameState.getDimensions());
    // TODO: prevent running into obstacles or out of water
}
