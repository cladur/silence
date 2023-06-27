#ifndef SILENCE_DIALOGUE_COLLIDER_DRAW_H
#define SILENCE_DIALOGUE_COLLIDER_DRAW_H

#include "base_system.h"
class DialogueColliderDrawSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_DIALOGUE_COLLIDER_DRAW_H
