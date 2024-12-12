#include "BoidEntity.h"

Ref<BoidProperties::BoidConfig> BoidProperties::BoidConfig::parseBoidConfig(Dictionary& config) {
    Ref<BoidConfig> props = memnew(BoidConfig);

    props->trailLen = config.get_or_add("trailLen", 0);
    props->groupRadius = config.get_or_add("groupRadius", 10);
    props->visionRadius = config.get_or_add("visionRadius", 10);
    props->maxSpeed = config.get_or_add("maxSpeed", 100);
    props->maxAccel = config.get_or_add("maxAccel", 100);
    props->dragPercent = config.get_or_add("dragPercent", 0);
    props->bouncePercent = config.get_or_add("bouncePercent", 0);
    props->separationWeight = config.get_or_add("separationWeight", 1);
    props->alignmentPercent = config.get_or_add("alignmentPercent", 0.2);
    props->cohesionWeight = config.get_or_add("cohesionWeight", 1);
    props->obstacleWeight = config.get_or_add("obstacleWeight", 1);
    props->tileWeights = config.get_or_add("materialWeights", Dictionary());
    props->entityWeights = config.get_or_add("entityWeights", Dictionary());
    props->food = config.get_or_add("food", Array());
    props->prey = config.get_or_add("prey", Array());

    return props;
}

BoidEntity::BoidEntity(StringName type, Ref<EntityProperties> properties, Vector2 position) : Entity(type, properties, position) {
    // TODO: make this more configurable
    velocity = Vector2::from_angle(UtilityFunctions::randf_range(0, Math_TAU)) * 10;

    // Make sure the properties is a BoidProperties
    (void) Object::cast_to<BoidProperties>(properties.ptr());
}

void BoidEntity::process(double delta, GameState& gameState) {
    Ref<BoidProperties::BoidConfig> config = Object::cast_to<BoidProperties>(properties.ptr())->boidConfig;

    Ref<MaterialProperties> curTile = gameState.getMaterialProperties(getCurrentTile(gameState));
    if (curTile->type == MaterialProperties::EMPTY) {
        position -= Vector2(0, delta * 10); // TODO: make gravity configurable
        return;
    } else if (curTile->isSolid()) {
        // TODO: don't hard-code this
        if (config->food.has(getCurrentTile(gameState).material)) {
            gameState.setTile(position.round(), Pixel{});
        } else {
            dead = true;
            return;
        }
    }

    Vector2i oldPos = position.round();
    Vector2 acceleration = Vector2();

    // Boid-related forces
    {
        Vector2 groupPos = Vector2();
        Vector2 groupVel = Vector2();
        int groupSize = 0;

        gameState.processNearbyEntities(position, config->visionRadius, [&] (Entity& e) {
            BoidEntity* boid = dynamic_cast<BoidEntity*>(&e);
            if (boid && boid->getType() == type) {
                if (boid->position.distance_squared_to(position) > config->groupRadius * config->groupRadius) { return; }

                groupPos += boid->position;
                groupVel += boid->velocity;
                groupSize++;

                Vector2 diff = boid->position - position;
                double distSquared = diff.length_squared();
                if (Math::is_zero_approx(distSquared)) { return; }

                // Separation
                acceleration += -diff * (1 / distSquared) * config->separationWeight; // Proportional to 1/distance
            } else {
                Vector2 diff = e.getPosition() - position;
                double distSquared = diff.length_squared();
                double dist = Math::sqrt(distSquared);

                if (Math::is_zero_approx(distSquared)) { return; }

                if (dist < 0.75 && config->prey.has(e.getType())) {
                    e.die();
                }

                if (config->entityWeights.has(e.getType())) {
                    double weight = config->entityWeights[e.getType()];
                    acceleration += diff * (1 / dist) * weight; // Proportional to 1
                }

            }
        });

        groupPos /= groupSize;
        groupVel /= groupSize;

        Vector2 diff = groupPos - position;
        double distSquared = diff.length_squared();
        double dist = Math::sqrt(distSquared);

        if (!Math::is_zero_approx(distSquared)) {
            // TODO: make attenuation controllable?

            // Cohesion
            acceleration += diff * config->cohesionWeight; // Proportional to distance

            // Alignment
            acceleration += (groupVel - velocity) * config->alignmentPercent; // Proportional to 1

            // Drag
            acceleration += -velocity * (1 - Math::pow(1 - config->dragPercent, delta));
        }
    }

    // Tile-related forces
    const Vector2i posI = position.round();
    for (int x = -config->visionRadius; x <= config->visionRadius; ++x) {
        for (int y = -config->visionRadius; y <= config->visionRadius; ++y) {
            Vector2i pos = posI + Vector2i(x, y);
            StringName mat = gameState.getTile(pos).material;
            Ref<MaterialProperties> properties = gameState.getMaterialProperties(mat);

            Vector2 diff = pos - position;
            double distSquared = diff.length_squared();
            if (distSquared > config->visionRadius * config->visionRadius) { continue; }
            double dist = Math::sqrt(distSquared);
            if (Math::is_zero_approx(distSquared)) { continue; }

            double weight = 0;
            if (config->tileWeights.has(mat)) {
                if (hasLineOfSightTo(gameState, pos)) {
                    weight = config->tileWeights[mat]; // Proportional to 1
                }
            } else if (!gameState.isInBounds(pos) || !properties->isFluid()) {
                weight = -config->obstacleWeight * (1 / distSquared ); // Proportional to 1/distance^2
            }

            acceleration += diff * (1 / dist) * weight;
        }
    }

    // Update velocity
    acceleration = acceleration.limit_length(config->maxAccel);
    velocity += acceleration * delta;
    velocity = velocity.limit_length(config->maxSpeed);

    // Eat food
    Vector2 newPos = position + velocity * delta;
    if (config->food.has(gameState.getTile(newPos.round()).material)) {
        gameState.setTile(newPos.round(), Pixel(""));
    }

    // Move and rebound
    bool collision = !move(velocity * delta, gameState);
    if (collision) {
        velocity *= -config->bouncePercent;
    }

    // Update trail
    Vector2 posi = position.round();
    if (posi != oldPos) {
        if (trail.size() >= config->trailLen) {
            trail.pop_front();
        }
        trail.push_back(oldPos);
    }
}

void BoidEntity::render(Ref<Image> image) {
    Vector2i pos = position.round();
    image->set_pixel(pos.x, pos.y, properties->color);

    Color trailColor = properties->color;
    trailColor.a *= 0.5;
    for (Vector2i trailPos : trail) {
        if (trailPos != pos) {
            image->set_pixel(trailPos.x, trailPos.y, trailColor);
        }
    }
}
