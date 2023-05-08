#ifndef SILENCE_ANIMATOR_H
#define SILENCE_ANIMATOR_H

#include "ecs/systems/base_system.h"
class Animation;
class SkinnedModel;
class Rig;
class AnimationSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;

	const int32_t MAX_BONE_COUNT = 512;

	explicit AnimationSystem(Animation &animation, SkinnedModel &model);

	void update_animation(float dt);

	void change_animation(Animation *animation);

	void change_model(SkinnedModel *model);

	void calculate_bone_transform();

	const std::vector<glm::mat4> &get_bone_matrices() const;

private:
	std::vector<glm::mat4> bone_matrices;
	Animation *current_animation;
	SkinnedModel *current_model;
	float current_time;
	float delta_time = 0.0f;
};

#endif //SILENCE_ANIMATOR_H
