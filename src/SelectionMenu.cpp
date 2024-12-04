#include "SelectionMenu.h"

void SelectionMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    ClassDB::bind_method(D_METHOD("on_button_pressed", "mat"), &SelectionMenu::on_button_pressed);
}

void SelectionMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    tiles = get_node<GridContainer>("%Tiles");
    assert(tiles);

    entities = get_node<GridContainer>("%Entities");
    assert(entities);

    selectedLabel = get_node<Label>("%Selected Label");
    assert(selectedLabel);

    auto selected = get_node<Panel>("%Selected Button");
    assert(selected);
    selectedStyle = selected->get_theme_stylebox("normal");
    assert(selectedStyle.is_valid());
    selected->queue_free();

    auto unselected = get_node<Panel>("%Unselected Button");
    assert(unselected);
    unselectedStyle = unselected->get_theme_stylebox("normal");
    assert(unselectedStyle.is_valid());
    unselected->queue_free();
}

void SelectionMenu::set_materials(const Materials& materials) {
    Array mats = materials.getAllMaterials();
    for (int i = 0; i < mats.size(); i++) {
        StringName mat = mats[i];
        Ref<MaterialProperties> props = materials.getProperties(mat);

        Button* button = memnew(Button);

        Ref<StyleBoxFlat> style = unselectedStyle->duplicate();
        style->set_bg_color(props->color);
        button->add_theme_stylebox_override("normal", style);
        button->add_theme_stylebox_override("hover", style);
        button->add_theme_stylebox_override("pressed", style);

        button->connect("pressed", Callable(this, "on_button_pressed").bind(mat));

        tiles->add_child(button);
    }
}
