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

    Vector2i gridSize = {50, 50};
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

    void setGridSize(Vector2i p_width);
    Vector2i getGridSize() const;
    void set_height(int p_height);
    int get_height() const;
    void setSimSpeed(double p_speed);
    double getSimSpeed() const;
    void setDefaultConfig(String p_file);
    String getDefaultConfig() const;

    void exportData(String p_file);
    void importData(String p_file);
    void importConfig(String p_file);

    void clearGrid();
};


#endif //GAMEMANAGER_H
