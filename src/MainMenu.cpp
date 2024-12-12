#include "MainMenu.h"

void MainMenu::_bind_methods() {
    UtilityFunctions::print("Registering class ", get_class_static());

    // properties
    ClassDB::bind_method(D_METHOD("get_game_scene"), &MainMenu::get_game_scene);
    ClassDB::bind_method(D_METHOD("set_game_scene", "game_scene"), &MainMenu::set_game_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "game_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
                 "set_game_scene", "get_game_scene");
    //
    // // start game button
    ClassDB::bind_method(D_METHOD("get_start_game_button"), &MainMenu::get_start_game_button);
    ClassDB::bind_method(D_METHOD("set_start_game_button", "start_game_button"),
                         &MainMenu::set_start_game_button);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "start_game_button", PROPERTY_HINT_NODE_TYPE, "Button"),
                 "set_start_game_button", "get_start_game_button");

    // quit game button
    ClassDB::bind_method(D_METHOD("get_quit_game_button"), &MainMenu::get_quit_game_button);
    ClassDB::bind_method(D_METHOD("set_quit_game_button", "quit_game_button"), &MainMenu::set_quit_game_button);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "quit_game_button", PROPERTY_HINT_NODE_TYPE, "Button"),
                 "set_quit_game_button", "get_quit_game_button");

    // other methods
    ClassDB::bind_method(D_METHOD("on_start_pressed"), &MainMenu::on_start_pressed);
    ClassDB::bind_method(D_METHOD("on_quit_pressed"), &MainMenu::on_quit_pressed);
}

void MainMenu::_ready() {
    if (Engine::get_singleton()->is_editor_hint()) return;

    DEV_ASSERT(this->gameScene.is_valid());

    this->startGameButton = this->get_node<Button>(startGameButtonPath);
    this->quitGameButton = this->get_node<Button>(quitGameButtonPath);

    DEV_ASSERT(this->startGameButton);
    DEV_ASSERT(this->quitGameButton);

    // bind signals
    Error err = this->startGameButton->connect("pressed", Callable(this, "on_start_pressed"));
    if (err != OK) {
        UtilityFunctions::printerr("Failed to connect start_game_button's pressed signal. Error: ", err);
    }

    err = this->quitGameButton->connect("pressed", Callable(this, "on_quit_pressed"));
    if (err != OK) {
        UtilityFunctions::printerr("Failed to connect quit_game_button's pressed signal. Error: ", err);
    }
}

void MainMenu::on_start_pressed() const {
    UtilityFunctions::print("Start game button pressed");
    if (this->gameScene.is_valid()) {
        UtilityFunctions::print("Changing scene to ", this->gameScene->get_path());
        get_tree()->change_scene_to_packed(this->gameScene);
    } else {
        UtilityFunctions::printerr("Game scene is not valid");
    }
}

void MainMenu::on_quit_pressed() const {
    UtilityFunctions::print("Quitting game");
    get_tree()->quit();
}
