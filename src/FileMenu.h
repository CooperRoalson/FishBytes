#ifndef FILEMENU_H
#define FILEMENU_H

#include "godot_includes.h"

class FileMenu : public Control {
    GDCLASS(FileMenu, Control)

    LineEdit* dataFileInput = nullptr;
    LineEdit* configFileInput = nullptr;

protected:
    static void _bind_methods();

public:
    void _ready() override;

    void onDataExportPressed();
    void onDataImportPressed();
    void onConfigImportPressed();
};

#endif //FILEMENU_H
