#ifndef SILENCE_ANIMATOR_H
#define SILENCE_ANIMATOR_H

#include "ecs/systems/base_system.h"
class AnimationInstance;
class Animation;
class SkinnedModelInstance;
struct Pose;

class AnimationSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ANIMATOR_H
