#ifndef MATERIALS_H
#define MATERIALS_H

#include "godot_includes.h"

struct MaterialProperties : public Resource {
    Color color = Color{"#000000", 0.0};
    String name = "Air";

    enum MaterialType : uint8_t {
        EMPTY,
        STATIC,
        GRAVITY,
        FLUID
    } type = EMPTY;

    static MaterialType typeFromString(const String& str) {
        if (str == "STATIC") {
            return STATIC;
        } else if (str == "GRAVITY") {
            return GRAVITY;
        } else if (str == "FLUID") {
            return FLUID;
        } else {
            return EMPTY;
        }
    }

    // Could add density or something

    MaterialProperties() = default;
    MaterialProperties(Color color, MaterialType type) : color(color), type(type) {}

    bool isFluid() const {
        return type == FLUID;
    }

    bool isSolid() const {
        return type == STATIC || type == GRAVITY;
    }
};


class Materials {
    Dictionary properties;

public:
    Materials(Dictionary materials);

    Array getAllMaterials() {
        return properties.keys();
    }

    Ref<MaterialProperties> getProperties(const StringName& mat) {
        return properties[mat];
    }
};


#endif //MATERIALS_H
