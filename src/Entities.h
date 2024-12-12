#ifndef ENTITIES_H
#define ENTITIES_H

#include "godot_includes.h"

struct EntityProperties : public Resource {
    Color color = Color{"#000000", 0.0};
    String name;
    enum EntityType {
        STATIC,
        BOID,
        BEHAVIOR
    }type = STATIC;

    EntityProperties() = default;

    static EntityType typeFromString(const String& str) {
        if (str == "STATIC") {
            return STATIC;
        } else if (str == "BOID") {
            return BOID;
        } else if (str == "BEHAVIOR") {
            return BEHAVIOR;
        } else {
            UtilityFunctions::printerr("Unknown entity type: ", str);
            return STATIC;
        }
    }
};


class Entities {
    Dictionary properties;

    Dictionary parseBoidConfigs(Dictionary boidData);
    Dictionary parseBehaviorTrees(Dictionary behaviorData);

public:
    Entities() : Entities(Dictionary(), Dictionary()) {}
    Entities(Dictionary entities, Dictionary entityConfig);

    Array getAllEntities() {
        return properties.keys();
    }

    Ref<EntityProperties> getProperties(const StringName& entity) {
        return properties[entity];
    }
};


#endif //ENTITIES_H
