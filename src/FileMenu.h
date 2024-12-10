#ifndef FILEMENU_H
#define FILEMENU_H

#include "godot_includes.h"

class FileMenu : public Control {
    GDCLASS(FileMenu, Control)


protected:
    static void _bind_methods();

public:
    void _ready() override;

};

#endif //FILEMENU_H
