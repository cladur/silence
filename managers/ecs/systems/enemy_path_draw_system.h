#ifndef SILENCE_ENEMY_PATH_DRAW_SYSTEM_H
#define SILENCE_ENEMY_PATH_DRAW_SYSTEM_H

#include "base_system.h"

class EnemyPathDraw : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ENEMY_PATH_DRAW_SYSTEM_H
