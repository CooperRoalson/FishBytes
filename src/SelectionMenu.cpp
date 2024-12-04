#include "SelectionMenu.h"

void SelectionMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    ClassDB::bind_method(D_METHOD("on_button_pressed", "button", "mat"), &SelectionMenu::onButtonPressed);
    ClassDB::bind_method(D_METHOD("on_button_hovered", "mat"), &SelectionMenu::onButtonHovered);
    ClassDB::bind_method(D_METHOD("on_button_exited"), &SelectionMenu::onButtonExited);
}

void SelectionMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    tiles = get_node<GridContainer>("%Tiles");
    DEV_ASSERT(tiles);

    entities = get_node<GridContainer>("%Entities");
    DEV_ASSERT(entities);

    selectedLabel = get_node<Label>("%Selected Label");
    DEV_ASSERT(selectedLabel);

    auto selected = get_node<Button>("%Selected Button");
    DEV_ASSERT(selected);
    selectedStyle = selected->get_theme_stylebox("normal");
    buttonSize = selected->get_custom_minimum_size();
    DEV_ASSERT(selectedStyle.is_valid());
    selected->queue_free();

    auto unselected = get_node<Button>("%Unselected Button");
    DEV_ASSERT(unselected);
    unselectedStyle = unselected->get_theme_stylebox("normal");
    DEV_ASSERT(unselectedStyle.is_valid());
    unselected->queue_free();
}

void SelectionMenu::setMaterials(Materials& materials) {
    Array mats = materials.getAllMaterials();
    for (int i = mats.size() - 1; i >= 0; i--) {
        StringName mat = mats[i];
        Ref<MaterialProperties> props = materials.getProperties(mat);

        Button* button = memnew(Button);

        Ref<StyleBoxFlat> style = Ref{memnew(StyleBoxFlat)};
        style->set_bg_color(props->color);
        button->add_theme_stylebox_override("normal", style);
        button->set_custom_minimum_size(buttonSize);

        button->connect("pressed", Callable(this, "on_button_pressed").bind(mat).bind(button));
        button->connect("mouse_entered", Callable(this, "on_button_hovered").bind(mat));
        button->connect("mouse_exited", Callable(this, "on_button_exited"));

        tiles->add_child(button);
        tiles->move_child(button, 0);

        onButtonPressed(button, mat);
    }
    onButtonExited();
}
