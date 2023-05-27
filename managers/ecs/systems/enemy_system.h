#ifndef SILENCE_ENEMY_SYSTEM_H
#define SILENCE_ENEMY_SYSTEM_H

#include "base_system.h"

class EnemySystem : public BaseSystem {

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
	void death(World &world, uint32_t entity);
};

#endif //SILENCE_ENEMY_SYSTEM_H
