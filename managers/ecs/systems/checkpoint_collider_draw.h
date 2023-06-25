#ifndef SILENCE_CHECKPOINT_COLLIDER_DRAW_H
#define SILENCE_CHECKPOINT_COLLIDER_DRAW_H

#include "base_system.h"
class CheckpointColliderDrawSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_CHECKPOINT_COLLIDER_DRAW_H
