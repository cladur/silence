#include "animation_manager.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "render/common/animation.h"
#include "render/common/skinned_model.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/quaternion.hpp>

AutoCVarFloat cvar_alpha("animation.alpha", "animation alpha value", 0.5f, CVarFlags::EditFloatDrag);

AnimationManager &AnimationManager::get() {
	static AnimationManager instance;
	return instance;
}

void AnimationManager::update_animation(AnimData &data, float dt) {
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
		if (!data.has_changed) {
			calculate_pose(data, data.model->current_pose);
		} else {
			data.has_changed = false;
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

	AnimData temp_data;
	AnimationInstance animation(new_animation_name.c_str());
	temp_data.animation = &animation;
	SkinnedModelInstance model = *entity_data.model;
	temp_data.model = &model;

	Pose p;
	p.matrices = std::vector<glm::mat4>(512, glm::mat4(1.0f));
	calculate_pose(temp_data, temp_data.model->current_pose);
	blend_poses(entity_data.model->current_pose, temp_data.model->current_pose, p, cvar_alpha.get());
	//	for (int i = 0; i < 512; ++i) {
	//		SPDLOG_INFO("temp {}", glm::to_string(temp_data.model->current_pose.matrices[i]));
	//		SPDLOG_INFO("entity {}", glm::to_string(entity_data.model->current_pose.matrices[i]));
	//		SPDLOG_INFO("result {}", glm::to_string(p.matrices[i]));
	//	}

	entity_data.model->current_pose = p;
	entity_data.animation->animation_handle = temp_data.animation->animation_handle;
	entity_data.has_changed = true;
}

void AnimationManager::calculate_pose(AnimData &data, Pose &result_pose) {
	ResourceManager &resource_manager = ResourceManager::get();
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);
	Animation &animation = resource_manager.get_animation(data.animation->animation_handle);

	if (result_pose.matrices.size() < MAX_BONE_COUNT) {
		SPDLOG_WARN("Result pose has not been initialized with {} matrices.", MAX_BONE_COUNT);
		assert(false);
		return;
	}
	Rig &rig = model.rig;
	std::vector<glm::mat4> global_matrices(rig.names.size());

	for (int32_t i = 0; i < model.rig.names.size(); ++i) {
		std::string node_name = rig.names[i];
		glm::mat4 node_transform = glm::translate(glm::mat4(1.0f), rig.positions[i]) * glm::toMat4(rig.rotations[i]);

		const auto &it = animation.channels.find(node_name);
		if (it != animation.channels.end()) {
			Channel &channel = it->second;

			channel.update(data.animation->current_time);
			node_transform = channel.local_transform;
		}

		int32_t parent_index = rig.parents[i];
		if (parent_index != -1) {
			global_matrices[i] = global_matrices[parent_index] * node_transform;
		} else {
			global_matrices[i] = node_transform;
		}

		const auto &joint = model.joint_map.find(node_name);
		if (joint != model.joint_map.end()) {
			int32_t index = joint->second.id;

			glm::mat4 offset =
					glm::translate(glm::mat4(1.0f), joint->second.translation) * glm::toMat4(joint->second.rotation);

			result_pose.matrices[index] = global_matrices[i] * offset;
		}
	}
}

void AnimationManager::blend_poses(const Pose &pose1, const Pose &pose2, Pose &result_pose, float alpha) {
	if (result_pose.matrices.size() < MAX_BONE_COUNT) {
		SPDLOG_WARN("Result pose has not been initialized with {} matrices.", MAX_BONE_COUNT);
		assert(false);
		return;
	}

	if (pose1.matrices.size() != pose2.matrices.size()) {
		SPDLOG_WARN("Poses have not equal matrices count");
		assert(false);
		return;
	}

	for (int32_t i = 0; i < pose1.matrices.size(); ++i) {
		slerp(pose1.matrices[i], pose2.matrices[i], result_pose.matrices[i], alpha);
	}
}

void AnimationManager::slerp(const glm::mat4 &m1, const glm::mat4 &m2, glm::mat4 &result, float alpha) {
	const glm::quat &rot1 = glm::toQuat(m1);
	const glm::quat &rot2 = glm::toQuat(m2);

	const glm::quat &result_rot = glm::normalize(glm::slerp(rot1, rot2, alpha));

	const glm::vec3 &pos1 = m1[3];
	const glm::vec3 &pos2 = m2[3];

	const glm::vec3 &result_pos = glm::lerp(pos1, pos2, alpha);

	result = glm::translate(glm::mat4(1.0f), result_pos) * glm::mat4_cast(result_rot);
}