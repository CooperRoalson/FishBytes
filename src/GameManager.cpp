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

    ClassDB::bind_method(D_METHOD("set_brush_radius", "p_radius"), &GameManager::set_brush_radius);
    ClassDB::bind_method(D_METHOD("get_brush_radius"), &GameManager::get_brush_radius);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "brush_radius", PROPERTY_HINT_RANGE, "1, 10, or_greater"), "set_brush_radius", "get_brush_radius");

    ClassDB::bind_method(D_METHOD("set_brush_density", "p_density"), &GameManager::set_brush_density);
    ClassDB::bind_method(D_METHOD("get_brush_density"), &GameManager::get_brush_density);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "brush_density", PROPERTY_HINT_RANGE, "0, 2, or_greater"), "set_brush_density", "get_brush_density");
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

void GameManager::set_brush_radius(int p_radius) { brushRadius = p_radius; }
int GameManager::get_brush_radius() const { return brushRadius; }

void GameManager::set_brush_density(double p_density) { brushDensity = p_density; }
double GameManager::get_brush_density() const { return brushDensity; }

void GameManager::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) {
        return;
    }

    gameState = std::make_unique<GameState>(width, height, Materials::defaultMaterials(), simSpeed);

    selectionMenu = get_node<SelectionMenu>("%SelectionMenu");
    DEV_ASSERT(selectionMenu);
    selectionMenu->setMaterials(gameState->getMaterials());

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

    DEV_ASSERT(image.is_valid());
    handleMouseInput(delta);
    gameState->process(delta);
    gameState->generateFrame(image);

    Ref<ImageTexture> texture = canvas->get_texture();
    DEV_ASSERT(texture.is_valid());
    texture->set_image(image);
}

void GameManager::handleMouseInput(double delta) {
    if (Input::get_singleton()->is_mouse_button_pressed(MOUSE_BUTTON_LEFT)) {
        Vector2 localMousePos = canvas->get_local_mouse_position();
        localMousePos = Vector2(localMousePos.x + 0.5, 1 - (localMousePos.y + 0.5));
        Vector2i mousePos = (localMousePos * Vector2(width, height)).round();

        if (mousePos.x < 0 || mousePos.y < 0 || mousePos.x >= width || mousePos.y >= height) {
            return;
        }

        for (int x = -brushRadius; x <= brushRadius; ++x) {
            for (int y = -brushRadius; y <= brushRadius; ++y) {
                if (x*x + y*y > brushRadius*brushRadius) {
                    continue;
                }

                if (UtilityFunctions::randf() < brushDensity * delta) {
                    Vector2i pos = mousePos + Vector2i(x, y);
                    gameState->setTile(pos, selectionMenu->getSelected());
                }
            }
        }
    }
}
