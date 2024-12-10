#ifndef ENTITIES_H
#define ENTITIES_H

#include "godot_includes.h"

struct EntityProperties : public Resource {
    Color color = Color{"#000000", 0.0};
    String name;
    enum EntityType {
        STATIC,
        BOID
    }type = STATIC;

    EntityProperties() = default;

    static EntityType typeFromString(const String& str) {
        if (str == "BOID") {
            return BOID;
        } else {
            return STATIC;
        }
    }
};


class Entities {
    Dictionary properties;

    Dictionary parseBoidConfigs(Dictionary boidData);

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
