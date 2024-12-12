#pragma once
#include "godot_includes.h"

class MainMenu : public Control {
    GDCLASS(MainMenu, Control)
    Button *startGameButton{};
    NodePath startGameButtonPath{};

    Button *quitGameButton{};
    NodePath quitGameButtonPath{};

    Ref<PackedScene> gameScene{};

    NodePath tempPath{NodePath("")};

protected:
    static void _bind_methods();

public:
    void _ready() override;

    [[nodiscard]] Ref<PackedScene> get_game_scene() const { return gameScene; }
    void set_game_scene(const Ref<PackedScene> &p_game_scene) { gameScene = p_game_scene; }

    [[nodiscard]] NodePath get_start_game_button() const { return startGameButtonPath; }
    void set_start_game_button(const NodePath &p_start_game_button) { startGameButtonPath = p_start_game_button; }

    [[nodiscard]] NodePath get_quit_game_button() const { return quitGameButtonPath; }
    void set_quit_game_button(const NodePath &p_quit_game_button) { quitGameButtonPath = p_quit_game_button; }

    void on_start_pressed() const;
    void on_quit_pressed() const;
};
