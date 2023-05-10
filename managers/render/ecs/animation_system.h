#ifndef SILENCE_ANIMATOR_H
#define SILENCE_ANIMATOR_H

#include "ecs/systems/base_system.h"
class Animation;
class SkinnedModelInstance;
class Bone;

struct AnimData {
	Animation *animation;
	SkinnedModelInstance *model;
	float current_time;
};

class AnimationSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;

	const int32_t MAX_BONE_COUNT = 512;

	void update_animation(AnimData &data, float dt);

	void calculate_bone_transform(AnimData &data, const Bone &bone, const glm::mat4 &parent_transform);
	std::unordered_map<Entity, AnimData> animation_map;
};

#endif //SILENCE_ANIMATOR_H
