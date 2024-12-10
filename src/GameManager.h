#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "GameState.h"
#include "godot_includes.h"
#include "SelectionMenu.h"
#include "FileMenu.h"

class GameManager : public Node2D {
    GDCLASS(GameManager, Node2D)

    SelectionMenu* selectionMenu = nullptr;
    FileMenu* fileMenu = nullptr;

    std::unique_ptr<GameState> gameState;

    int width = 50, height = 50;
    double simSpeed = 14.0;

    bool isMouseDown = false;

    MeshInstance2D* canvas = nullptr;
    Ref<Image> image;

    String defaultConfig = "res://config.json";

    void handleMouseInput(double delta);

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _process(double p_delta) override;
    void _physics_process(double delta) override;

    void set_width(int p_width);
    int get_width() const;
    void set_height(int p_height);
    int get_height() const;
    void set_sim_speed(double p_speed);
    double get_sim_speed() const;
    void set_default_config(String p_file);
    String get_default_config() const;

    void export_data(String p_file);
    void import_data(String p_file);
    void import_config(String p_file);
};


#endif //GAMEMANAGER_H
