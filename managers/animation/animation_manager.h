#ifndef SILENCE_ANIMATION_MANAGER_H
#define SILENCE_ANIMATION_MANAGER_H

class AnimationInstance;
class Animation;
class SkinnedModelInstance;
struct Pose;

struct AnimData {
	AnimationInstance *animation;
	SkinnedModelInstance *model;
	bool has_changed = false;
};

class AnimationManager {
public:
	static AnimationManager &get();

	void update_animation(AnimData &data, float dt);
	void change_animation(Entity entity, const std::string &new_animation_name);

	void calculate_pose(AnimData &data, Pose &result_pose);
	// Blend pose1 with pose2 by alpha value into result_pose
	void blend_poses(const Pose &pose1, const Pose &pose2, Pose &result_pose, float alpha);
	// slerp matrices with position and rotation
	void slerp(const glm::mat4 &m1, const glm::mat4 &m2, glm::mat4 &result, float alpha);

	const int32_t MAX_BONE_COUNT = 512;
	std::unordered_map<Entity, AnimData> animation_map;
};

#endif //SILENCE_ANIMATION_MANAGER_H
