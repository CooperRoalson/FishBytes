#include "GameManager.h"

void GameManager::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    ClassDB::bind_method(D_METHOD("set_width", "p_width"), &GameManager::set_width);
    ClassDB::bind_method(D_METHOD("get_width"), &GameManager::get_width);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "width", PROPERTY_HINT_RANGE, "1, 100, or_greater"), "set_width", "get_width");

    ClassDB::bind_method(D_METHOD("set_height", "p_height"), &GameManager::set_height);
    ClassDB::bind_method(D_METHOD("get_height"), &GameManager::get_height);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "height", PROPERTY_HINT_RANGE, "1, 100, or_greater"), "set_height", "get_height");

    ClassDB::bind_method(D_METHOD("set_sim_speed", "p_speed"), &GameManager::set_sim_speed);
    ClassDB::bind_method(D_METHOD("get_sim_speed"), &GameManager::get_sim_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sim_speed", PROPERTY_HINT_RANGE, "0.1, 20, or_greater"), "set_sim_speed", "get_sim_speed");

    ClassDB::bind_method(D_METHOD("set_config_file", "p_file"), &GameManager::set_config_file);
    ClassDB::bind_method(D_METHOD("get_config_file"), &GameManager::get_config_file);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "config_file", PROPERTY_HINT_FILE), "set_config_file", "get_config_file");
}

void GameManager::set_width(int p_width) { width = p_width; }
int GameManager::get_width() const { return width; }

void GameManager::set_height(int p_height) { height = p_height; }
int GameManager::get_height() const { return height; }

void GameManager::set_sim_speed(double p_speed) {
    simSpeed = p_speed;
    if (gameState) { gameState->setSimSpeed(simSpeed); }
}
double GameManager::get_sim_speed() const { return simSpeed; }

void GameManager::set_config_file(String p_file) { configFile = p_file; }
String GameManager::get_config_file() const { return configFile; }

void GameManager::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    Ref<JSON> json = ResourceLoader::get_singleton()->load(configFile, "JSON");
    Dictionary config = json.is_valid() ? Dictionary(json->get_data()) : Dictionary();
    Dictionary materials = config.get_or_add("materials", Dictionary());
    Dictionary entities = config.get_or_add("entities", Dictionary());
    Dictionary entityConfig = config.get_or_add("entityConfig", Dictionary());

    gameState = std::make_unique<GameState>(width, height, Materials(materials), Entities(entities, entityConfig), simSpeed);

    selectionMenu = get_node<SelectionMenu>("%SelectionMenu");
    DEV_ASSERT(selectionMenu);
    selectionMenu->setContents(gameState->getMaterials(), gameState->getEntities());

    canvas = get_node<MeshInstance2D>("%Canvas");
    DEV_ASSERT(canvas);

    PackedByteArray arr;
    arr.resize(4);

    image.instantiate();
    image->set_data(1,1, false, Image::FORMAT_RGBA8, arr);
    image->resize(width, height);

}

void GameManager::_physics_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    handleMouseInput(delta);
    gameState->process(delta);
}

void GameManager::_process(double delta) {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    DEV_ASSERT(image.is_valid());
    gameState->generateFrame(image);
    Ref<ImageTexture> texture = canvas->get_texture();
    DEV_ASSERT(texture.is_valid());
    texture->set_image(image);
}

void GameManager::handleMouseInput(double delta) {
    bool wasMouseDown = isMouseDown;
    isMouseDown = Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT);

    if (isMouseDown) {
        double brushRadius = selectionMenu->getBrushRadius() - 1;
        double brushDensity = selectionMenu->getBrushDensity();
        bool autoFill = brushDensity >= brushRadius || brushDensity > 3.5;
        StringName type = selectionMenu->getSelected();

        Vector2 localMousePos = canvas->get_local_mouse_position();
        localMousePos = Vector2(localMousePos.x + 0.5, 1 - (localMousePos.y + 0.5));
        Vector2i mousePos = (localMousePos * Vector2(width, height)).round();

        if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x >= width || mousePos.y >= height) {
            return;
        }

        if (selectionMenu->isEntitySelected()) {
            if (!wasMouseDown) {
                gameState->spawnEntity(mousePos, type);
            }
            return;
        }

        for (int x = -brushRadius; x <= brushRadius; ++x) {
            for (int y = -brushRadius; y <= brushRadius; ++y) {
                if (x*x + y*y > brushRadius*brushRadius) {
                    continue;
                }

                if (autoFill || UtilityFunctions::randf() < brushDensity * delta) {
                    Vector2i pos = mousePos + Vector2i(x, y);
                    gameState->setTile(pos, type);
                }
            }
        }
    }
}
