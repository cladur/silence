#ifndef SILENCE_ANIMATION_MANAGER_H
#define SILENCE_ANIMATION_MANAGER_H

#include "pose.h"
class AnimationInstance;
class Animation;
class SkinnedModelInstance;
struct Xform;
class World;

struct AnimData {
	AnimationInstance *animation;
	SkinnedModelInstance *model;
	Pose local_pose;
	Pose model_pose;
	//	float blend_time_ms = 400.0f;
	bool has_changed = false;
};

class AnimationManager {
public:
	static AnimationManager &get();
	AnimationManager(const AnimationManager &) = delete;

	void update_pose(AnimData &data, float dt);
	void change_animation(Entity entity, const std::string &new_animation_name);

	void local_to_model(AnimData &data);

	void model_to_final(AnimData &data);
	// Blend pose1 with pose2 by alpha value into result_pose
	void blend_poses(const Pose &pose1, const Pose &pose2, Pose &result_pose, float alpha);
	// slerp xforms with position and rotation
	void slerp(const Xform &m1, const Xform &m2, Xform &result, float alpha);

	glm::mat4 get_bone_transform(Entity holder, std::string &bone_name);

	void attach_to_entity(World &world, Entity holder, Entity attached, const std::string &bone_name);
	void detach_entity(World &world, Entity attached);
	const int32_t MAX_BONE_COUNT = 512;
	std::unordered_map<Entity, AnimData> animation_map;

	static inline float calculate_uniform_s(float alpha) {
		float sqt = alpha * alpha;
		return sqt / (2.0f * (sqt - alpha) + 1.0f);
	}

private:
	AnimationManager() = default;
};

#endif //SILENCE_ANIMATION_MANAGER_H
