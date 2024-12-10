#include "FileMenu.h"

void FileMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    ClassDB::bind_method(D_METHOD("on_data_export_pressed"), &FileMenu::onDataExportPressed);
    ClassDB::bind_method(D_METHOD("on_data_import_pressed"), &FileMenu::onDataImportPressed);
    ClassDB::bind_method(D_METHOD("on_config_import_pressed"), &FileMenu::onConfigImportPressed);

    ADD_SIGNAL(MethodInfo("export_data", PropertyInfo(Variant::STRING, "file")));
    ADD_SIGNAL(MethodInfo("import_data", PropertyInfo(Variant::STRING, "file")));
    ADD_SIGNAL(MethodInfo("import_config", PropertyInfo(Variant::STRING, "file")));
}

void FileMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    dataFileInput = get_node<LineEdit>("%Data File Input");
    dataExportButton = get_node<Button>("%Data Export Button");
    dataImportButton = get_node<Button>("%Data Import Button");
    configFileInput = get_node<LineEdit>("%Config File Input");
    configImportButton = get_node<Button>("%Config Import Button");

    DEV_ASSERT(dataFileInput);
    DEV_ASSERT(dataExportButton);
    DEV_ASSERT(dataImportButton);
    DEV_ASSERT(configFileInput);
    DEV_ASSERT(configImportButton);

    dataExportButton->connect("pressed", Callable(this, "on_data_export_pressed"));
    dataImportButton->connect("pressed", Callable(this, "on_data_import_pressed"));
    configImportButton->connect("pressed", Callable(this, "on_config_import_pressed"));
}

void FileMenu::onDataExportPressed() {
    emit_signal("export_data", dataFileInput->get_text());
}

void FileMenu::onDataImportPressed() {
    emit_signal("import_data", dataFileInput->get_text());
}

void FileMenu::onConfigImportPressed() {
    emit_signal("import_config", configFileInput->get_text());
}
