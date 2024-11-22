#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "GameState.h"
#include "godot_includes.h"
#include "Materials.h"

class GameManager : public Node2D {
    GDCLASS(GameManager, Node2D)

    std::unique_ptr<GameState> gameState;

    int width = 50, height = 50;
    double simSpeed = 8.0;

    Ref<ImageTexture> texture;
    Ref<Image> image;

protected:
    static void _bind_methods();

public:
    void _ready() override;
    void _physics_process(double delta) override;

    void set_width(int p_width);
    int get_width() const;
    void set_height(int p_height);
    int get_height() const;
    void set_sim_speed(double p_speed);
    double get_sim_speed() const;
};


#endif //GAMEMANAGER_H
