#ifndef SELECTIONMENU_H
#define SELECTIONMENU_H

#include "godot_includes.h"
#include "Materials.h"
#include "Entities.h"

class SelectionMenu : public Control {
    GDCLASS(SelectionMenu, Control)

    GridContainer* tileGrid = nullptr;
    GridContainer* entityGrid = nullptr;
    Label* selectedLabel = nullptr;

    Vector2i buttonSize;
    Ref<StyleBoxFlat> selectedStyle;
    Ref<StyleBoxFlat> unselectedStyle;

    Button* selectedButton = nullptr;
    StringName selected = "";
    String selectedName = "Air";
    bool entitySelected = false;

    Slider* brushRadiusSlider = nullptr;
    Slider* brushDensitySlider = nullptr;

protected:
    static void _bind_methods();

public:
    void _ready() override;

    void clearMenu();

    void setContents(Materials& materials, Entities& entities);

    void onButtonPressed(Button* button, StringName clicked, String clickedName, bool isEntity) {
        Ref<StyleBoxFlat> style = selectedStyle->duplicate();
        Ref<StyleBoxFlat> oldStyle = button->get_theme_stylebox("normal");
        style->set_bg_color(oldStyle->get_bg_color());
        button->add_theme_stylebox_override("normal", style);
        button->add_theme_stylebox_override("hover", style);
        button->add_theme_stylebox_override("pressed", style);

        if (selectedButton) {
            style = unselectedStyle->duplicate();
            oldStyle = selectedButton->get_theme_stylebox("normal");
            style->set_bg_color(oldStyle->get_bg_color());
            selectedButton->add_theme_stylebox_override("normal", style);
            selectedButton->add_theme_stylebox_override("hover", style);
            selectedButton->add_theme_stylebox_override("pressed", style);
        }

        selectedButton = button;
        selected = clicked;
        selectedName = clickedName;
        entitySelected = isEntity;
    }

    void onButtonHovered(StringName hovered, String hoveredName) {
        selectedLabel->set_text(hoveredName);
    }

    void onButtonExited() {
        selectedLabel->set_text(selectedName);
    }

    double getBrushRadius() const {
        return brushRadiusSlider->get_value();
    }

    double getBrushDensity() const {
        return brushDensitySlider->get_value();
    }

    StringName getSelected() const { return selected; }

    bool isEntitySelected() const { return entitySelected; }

    void undo() { emit_signal("undo"); }
    void clearGrid() { emit_signal("clear_grid"); }
};


#endif //SELECTIONMENU_H
