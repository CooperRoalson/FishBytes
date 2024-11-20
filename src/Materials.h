#ifndef MATERIALS_H
#define MATERIALS_H

#include "godot_includes.h"

struct MaterialProperties : public Resource {
    Color color = Color{"#000000", 0.0};

    enum MaterialType : uint8_t {
        EMPTY,
        STATIC,
        GRAVITY,
        FLUID,
    } type = EMPTY;

    // Could add density or something

protected:
    MaterialProperties() = default;
    MaterialProperties(Color color, MaterialType type) : color(color), type(type) {}

public:
    static Ref<MaterialProperties> makeMaterial(Color c, MaterialType type) {
        return {memnew(MaterialProperties(c, type))};
    }

    static Ref<MaterialProperties> makeEmptyMaterial() {
        return {memnew(MaterialProperties())};
    }
};


class Materials {
    Dictionary properties;

public:

    Ref<MaterialProperties> getProperties(const StringName& mat) {
        return properties[mat];
    }

    bool isFluid(const StringName& mat) {
        auto type = getProperties(mat)->type;
        return type == MaterialProperties::FLUID || type == MaterialProperties::EMPTY;
    }

    static Materials defaultMaterials() {
        Materials mat;

        mat.properties[""]      = MaterialProperties::makeEmptyMaterial();
        mat.properties["water"] = MaterialProperties::makeMaterial(Color{"#4d8cc4", 0.3}, MaterialProperties::FLUID);
        mat.properties["wood"]  = MaterialProperties::makeMaterial(Color{"#543a06", 1.0}, MaterialProperties::STATIC);
        mat.properties["sand"]  = MaterialProperties::makeMaterial(Color{"#d1a62e", 1.0}, MaterialProperties::GRAVITY);

        return mat;
    }
};


#endif //MATERIALS_H
