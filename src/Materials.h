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

    [[nodiscard]] bool isFluid() const {
        return type == FLUID;
    }

    [[nodiscard]] bool isSolid() const {
        return type == STATIC || type == GRAVITY;
    }
};


class Materials {
    Dictionary properties;
    Ref<MaterialProperties> missingMaterial;

public:
    Materials() : Materials(Dictionary()) {}
    explicit Materials(Dictionary materials);

    [[nodiscard]] Array getAllMaterials() const {
        return properties.keys();
    }

    Ref<MaterialProperties> getProperties(const StringName& mat) {
        Ref<MaterialProperties> props = properties[mat];
        return props.is_valid() ? props : missingMaterial;
    }
};


#endif //MATERIALS_H
