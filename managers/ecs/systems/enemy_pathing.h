#ifndef SILENCE_ENEMY_PATHING_H
#define SILENCE_ENEMY_PATHING_H

#include "base_system.h"

class EnemyPathing : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ENEMY_PATHING_H
