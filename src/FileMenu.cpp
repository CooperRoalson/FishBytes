#include "FileMenu.h"

void FileMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());
}

void FileMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

}
