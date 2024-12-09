#include "Entities.h"
#include "BoidEntity.h"

Dictionary Entities::parseBoidConfigs(Dictionary boidData) {
    Dictionary boidConfig;
    Array ids = boidData.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary config = boidData[id];

        Ref<BoidProperties::BoidConfig> props = memnew(BoidProperties::BoidConfig);
        boidConfig[id] = props;

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
    }
    return boidConfig;
}

Entities::Entities(Dictionary entities, Dictionary entityConfig) {
    Dictionary boidConfigs = parseBoidConfigs(entityConfig.get_or_add("boids", Dictionary()));

    Array ids = entities.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary entity = entities[id];

        EntityProperties::EntityType type = EntityProperties::typeFromString(entity.get_or_add("type", "STATIC"));

        Ref<EntityProperties> props;
        switch (type) {
            case EntityProperties::STATIC: {
                props = Ref(memnew(EntityProperties));
                break;
            }
            case EntityProperties::BOID: {
                Ref<BoidProperties> boid = memnew(BoidProperties);
                props = boid;

                String config = entity.get_or_add("config", "");
                boid->boidConfig = Ref<BoidProperties::BoidConfig>(boidConfigs.get_or_add(config, Ref<BoidProperties::BoidConfig>()));
                break;
            }
        }

        properties[id] = props;

        props->color = Color(entity.get_or_add("color", Color("#000000")));
        props->color.a = entity.get_or_add("alpha", 1.0);
        props->type = type;
        props->name = entity.get_or_add("name", id.capitalize());
    }
}
