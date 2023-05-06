#ifndef SILENCE_ANIMATOR_H
#define SILENCE_ANIMATOR_H

class Animation;
class Rig;
class AnimationManager {
public:
	const int32_t MAX_BONE_COUNT = 512;

	explicit AnimationManager(Animation &animation);

	void update_animation(float dt);

	void change_animation(Animation *animation);

	void calculate_bone_transform(const Rig &rig, glm::mat4 parent_transform);

	const std::vector<glm::mat4> &get_bone_matrices() const;

private:
	std::vector<glm::mat4> bone_matrices;
	Animation *current_animation;
	float current_time;
	float delta_time = 0.0f;
};

#endif //SILENCE_ANIMATOR_H
