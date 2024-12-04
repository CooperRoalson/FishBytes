#ifndef ENTITIES_H
#define ENTITIES_H

#include "godot_includes.h"

struct EntityProperties : public Resource {
    Color color = Color{"#000000", 0.0};
    enum EntityType {
        STATIC,
        BOID
    }type = STATIC;

    EntityProperties() = default;
    EntityProperties(Color color, EntityType type) : color(color), type(type) {}
};


class Entities {
    Dictionary properties;

public:
    Array getAllEntities() {
        return properties.keys();
    }

    Ref<EntityProperties> getProperties(const StringName& entity) {
        return properties[entity];
    }

    static Entities defaultEntities();
};


#endif //ENTITIES_H
