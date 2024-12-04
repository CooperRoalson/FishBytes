#ifndef SELECTIONMENU_H
#define SELECTIONMENU_H

#include "godot_includes.h"
#include "Materials.h"

class SelectionMenu : public Control {
    GDCLASS(SelectionMenu, Control)

    GridContainer* tiles = nullptr;
    GridContainer* entities = nullptr;
    Label* selectedLabel = nullptr;

    Ref<StyleBoxFlat> selectedStyle;
    Ref<StyleBoxFlat> unselectedStyle;

    StringName selected;

protected:
    static void _bind_methods();

public:
    void _ready() override;

    StringName get_selected() const { return selected; }

    void set_materials(const Materials& materials);

    void on_button_pressed(StringName mat) {
        selected = mat;
        selectedLabel->set_text(mat);
    }
};


#endif //SELECTIONMENU_H
