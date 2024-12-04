#ifndef MATERIALS_H
#define MATERIALS_H

#include "godot_includes.h"

struct MaterialProperties : public Resource {
    Color color = Color{"#000000", 0.0};

    enum MaterialType : uint8_t {
        EMPTY,
        STATIC,
        GRAVITY,
        FLUID
    } type = EMPTY;

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
    Array getAllMaterials() {
        return properties.keys();
    }

    Ref<MaterialProperties> getProperties(const StringName& mat) {
        return properties[mat];
    }

    static Materials defaultMaterials() {
        Materials mat;

        mat.properties[""]      = {memnew(MaterialProperties)};
        mat.properties["water"] = {memnew(MaterialProperties(Color{"#4d8cc4", 0.3}, MaterialProperties::FLUID))};
        mat.properties["wood"]  = {memnew(MaterialProperties(Color{"#543a06", 1.0}, MaterialProperties::STATIC))};
        mat.properties["sand"]  = {memnew(MaterialProperties(Color{"#d1a62e", 1.0}, MaterialProperties::GRAVITY))};
        mat.properties["food"]  = {memnew(MaterialProperties(Color{"#07ab22", 1.0}, MaterialProperties::GRAVITY))};

        return mat;
    }
};


#endif //MATERIALS_H
