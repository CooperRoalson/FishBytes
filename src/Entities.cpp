#include "Entities.h"

#include "BehaviorEntity.h"
#include "BoidEntity.h"

Dictionary Entities::parseBoidConfigs(Dictionary boidData) {
    Dictionary boidConfigs;
    Array ids = boidData.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary config = boidData[id];
        boidConfigs[id] = BoidProperties::BoidConfig::parseBoidConfig(config);
    }
    return boidConfigs;
}

Dictionary Entities::parseBehaviorTrees(Dictionary behaviorData) {
    Dictionary behaviorTrees;
    Array ids = behaviorData.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary config = behaviorData[id];
        behaviorTrees[id] = BehaviorTree::parseBehaviorTree(config);
    }
    return behaviorTrees;
}


Entities::Entities(Dictionary entities, Dictionary entityConfig) {
    Dictionary boidConfigs = parseBoidConfigs(entityConfig.get_or_add("boids", Dictionary()));
    Dictionary behaviorTrees = parseBehaviorTrees(entityConfig.get_or_add("behaviorTrees", Dictionary()));

    Array ids = entities.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary entity = entities[id];

        if (!entity.has("type")) {
            UtilityFunctions::printerr("Entity missing type: ", id);
            continue;
        }
        EntityProperties::EntityType type = EntityProperties::typeFromString(entity["type"]);


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
                if (!boidConfigs.has(config)) {
                    UtilityFunctions::printerr("Boid config not found: ", config);
                    continue;
                }
                boid->boidConfig = boidConfigs[config];
                break;
            }
            case EntityProperties::BEHAVIOR: {
                Ref<BehaviorProperties> behavior = memnew(BehaviorProperties);
                props = behavior;

                String config = entity.get_or_add("config", "");
                if (!behaviorTrees.has(config)) {
                    UtilityFunctions::printerr("Behavior tree not found: ", config);
                    continue;
                }
                behavior->tree = behaviorTrees[config];
                behavior->defaultBlackboardOverrides = entity.get_or_add("blackboard", Dictionary());
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
