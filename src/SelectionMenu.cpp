#include "SelectionMenu.h"

void SelectionMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    ClassDB::bind_method(D_METHOD("on_button_pressed", "button", "clicked", "clickedName", "isEntity"), &SelectionMenu::onButtonPressed);
    ClassDB::bind_method(D_METHOD("on_button_hovered", "hovered", "hoveredName"), &SelectionMenu::onButtonHovered);
    ClassDB::bind_method(D_METHOD("on_button_exited"), &SelectionMenu::onButtonExited);
}

void SelectionMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    tileGrid = get_node<GridContainer>("%Tiles");
    DEV_ASSERT(tileGrid);

    entityGrid = get_node<GridContainer>("%Entities");
    DEV_ASSERT(entityGrid);

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

    brushRadiusSlider = get_node<Slider>("%Radius Slider");
    DEV_ASSERT(brushRadiusSlider);

    brushDensitySlider = get_node<Slider>("%Density Slider");
    DEV_ASSERT(brushDensitySlider);
}

void SelectionMenu::setContents(Materials& materials, Entities& entities) {
    Array arr = materials.getAllMaterials();
    int numMats = arr.size();
    arr.append_array(entities.getAllEntities());
    for (int i = arr.size() - 1; i >= 0; i--) {
        StringName id = arr[i];
        bool isEntity = i >= numMats;
        Color color = isEntity ? entities.getProperties(id)->color : materials.getProperties(id)->color;
        String name = isEntity ? entities.getProperties(id)->name : materials.getProperties(id)->name;

        Button* button = memnew(Button);

        Ref<StyleBoxFlat> style = Ref{memnew(StyleBoxFlat)};
        style->set_bg_color(color);
        button->add_theme_stylebox_override("normal", style);
        button->set_custom_minimum_size(buttonSize);

        button->connect("pressed", Callable(this, "on_button_pressed").bind(isEntity).bind(name).bind(id).bind(button));
        button->connect("mouse_entered", Callable(this, "on_button_hovered").bind(name).bind(id));
        button->connect("mouse_exited", Callable(this, "on_button_exited"));

        Node* parent = isEntity ? entityGrid : tileGrid;
        parent->add_child(button);
        parent->move_child(button, 0);

        onButtonPressed(button, id, name, isEntity);
    }
    onButtonExited();
}
