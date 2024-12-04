#ifndef SELECTIONMENU_H
#define SELECTIONMENU_H

#include "godot_includes.h"
#include "Materials.h"

class SelectionMenu : public Control {
    GDCLASS(SelectionMenu, Control)

    GridContainer* tiles = nullptr;
    GridContainer* entities = nullptr;
    Label* selectedLabel = nullptr;

    Vector2i buttonSize;
    Ref<StyleBoxFlat> selectedStyle;
    Ref<StyleBoxFlat> unselectedStyle;

    Button* selectedButton = nullptr;
    StringName selected;

    Slider* brushRadiusSlider = nullptr;
    Slider* brushDensitySlider = nullptr;

    String getMatName(StringName mat) {
        return mat.is_empty() ? "Air" : mat.capitalize();
    }

protected:
    static void _bind_methods();

public:
    void _ready() override;

    StringName getSelected() const { return selected; }

    void setMaterials(Materials& materials);

    void onButtonPressed(Button* button, StringName mat) {
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
        selected = mat;
    }

    void onButtonHovered(StringName mat) {
        selectedLabel->set_text(getMatName(mat));
    }

    void onButtonExited() {
        selectedLabel->set_text(getMatName(selected));
    }

    double getBrushRadius() const {
        return brushRadiusSlider->get_value();
    }

    double getBrushDensity() const {
        return brushDensitySlider->get_value();
    }
};


#endif //SELECTIONMENU_H
