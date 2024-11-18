#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include "godot_includes.h"

class GameManager : public Node2D {
    GDCLASS(GameManager, Node2D)

protected:
    static void _bind_methods();

public:
    GameManager();

};


#endif //GAMEMANAGER_H
