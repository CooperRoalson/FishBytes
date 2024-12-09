#include "Materials.h"

Materials::Materials(Dictionary materials) {
    properties[""]      = {memnew(MaterialProperties)};

    Array ids = materials.keys();
    for (int i = 0; i < ids.size(); ++i) {
        String id = ids[i];
        Dictionary mat = materials[id];

        Ref<MaterialProperties> props = memnew(MaterialProperties);
        properties[id] = props;

        props->color = Color(mat.get_or_add("color", Color("#000000")));
        props->color.a = mat.get_or_add("alpha", 1.0);
        props->type = MaterialProperties::typeFromString(mat.get_or_add("type", "STATIC"));
        props->name = mat.get_or_add("name", id.capitalize());
    }
}
