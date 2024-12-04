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


void GameManager::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    gameState = std::make_unique<GameState>(width, height, Materials::defaultMaterials(), simSpeed);

    selectionMenu = get_node<SelectionMenu>("%SelectionMenu");
    DEV_ASSERT(selectionMenu);
    selectionMenu->setMaterials(gameState->getMaterials());

    auto canvas = get_node<MeshInstance2D>("%Canvas");
    DEV_ASSERT(canvas);
    texture = canvas->get_texture();
    DEV_ASSERT(texture.is_valid());

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

    DEV_ASSERT(image.is_valid());
    gameState->process(delta);
    gameState->generateFrame(image);
    texture->set_image(image);
}
