#include "animation_manager.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "render/common/animation.h"
#include "render/common/skinned_model.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <glm/gtx/quaternion.hpp>

AutoCVarFloat cvar_alpha("animation.alpha", "animation alpha value", 0.5f, CVarFlags::EditFloatDrag);

AnimationManager &AnimationManager::get() {
	static AnimationManager instance;
	return instance;
}

void AnimationManager::update_pose(AnimData &data, float dt) {
	if (!data.animation) { // todo: this check is not work properly i think, check it
		SPDLOG_WARN("Data animation is not set.");
		assert(false);
		return;
	}
	ResourceManager &resource_manager = ResourceManager::get();
	Animation &animation = resource_manager.get_animation(data.animation->animation_handle);
	float &current_time = data.animation->current_time;
	current_time += static_cast<float>(animation.get_ticks_per_second()) * dt;

	if (current_time < animation.get_duration() || data.animation->is_looping) {
		current_time = fmod(current_time, static_cast<float>(animation.get_duration()));
		animation.sample(data, data.model->current_pose);
		if (data.has_changed) {
			Animation &next_animation = resource_manager.get_animation(data.animation->next_animation);
			Pose next_pose;
			current_time = 0.0f;
			next_animation.sample(data, next_pose);
			blend_poses(data.model->current_pose, next_pose, data.model->current_pose, cvar_alpha.get());
			data.has_changed = false;
			data.animation->animation_handle = data.animation->next_animation;
		}
		local_to_model(data);
	}
}

void AnimationManager::local_to_model(AnimData &data) {
	ResourceManager &resource_manager = ResourceManager::get();
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);

	Rig &rig = model.rig;

	std::vector<Xform> &pose_matrices = data.model->current_pose.xfroms;
	std::vector<glm::mat4> global_matrices(pose_matrices.size());

	for (int32_t i = 0; i < model.rig.names.size(); ++i) {
		std::string &node_name = rig.names[i];

		int32_t parent_index = rig.parents[i];
		if (parent_index != -1) {
			global_matrices[i] = global_matrices[parent_index] * glm::mat4(pose_matrices[i]);
		} else {
			global_matrices[i] = glm::mat4(pose_matrices[i]);
		}

		const auto &joint = model.joint_map.find(node_name);
		if (joint != model.joint_map.end()) {
			int32_t index = joint->second.id;

			glm::mat4 offset =
					glm::translate(glm::mat4(1.0f), joint->second.translation) * glm::toMat4(joint->second.rotation);

			data.model->bone_matrices[index] = global_matrices[i] * offset;
		}
	}
}

void AnimationManager::change_animation(Entity entity, const std::string &new_animation_name) {
	const auto &item = animation_map.find(entity);
	if (item == animation_map.end()) {
		SPDLOG_WARN("Entity {} not found in map.", entity);
		assert(false);
		return;
	}
	ResourceManager &resource_manager = ResourceManager::get();

	AnimData &entity_data = item->second;
	entity_data.animation->next_animation = resource_manager.get_animation_handle(new_animation_name);
	entity_data.has_changed = true;
}

void AnimationManager::blend_poses(const Pose &pose1, const Pose &pose2, Pose &result_pose, float alpha) {
	if (pose1.xfroms.size() != pose2.xfroms.size() || pose1.xfroms.size() != result_pose.xfroms.size()) {
		SPDLOG_WARN("Poses have not equal xfroms count");
		assert(false);
		return;
	}

	for (int32_t i = 0; i < result_pose.xfroms.size(); ++i) {
		slerp(pose1.xfroms[i], pose2.xfroms[i], result_pose.xfroms[i], alpha);
	}
}

void AnimationManager::slerp(const Xform &m1, const Xform &m2, Xform &result, float alpha) {
	result.rotation = glm::normalize(glm::slerp(m1.rotation, m2.rotation, alpha));
	result.translation = glm::mix(m1.translation, m2.translation, alpha);
}