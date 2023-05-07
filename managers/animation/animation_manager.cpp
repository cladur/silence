#include "animation_manager.h"
#include "animation.h"
#include "joint.h"
#include "render/common/skinned_model.h"
#include <glm/gtx/quaternion.hpp>

AnimationManager::AnimationManager(Animation &animation, SkinnedModel &model) {
	current_time = 0.0f;
	current_animation = &animation;
	current_model = &model;

	bone_matrices.reserve(MAX_BONE_COUNT);

	for (int32_t i = 0; i < MAX_BONE_COUNT; ++i) {
		bone_matrices.emplace_back(1.0f);
	}
}
void AnimationManager::update_animation(float dt) {
	delta_time = dt;
	if (current_animation) {
		current_time += static_cast<float>(current_animation->get_ticks_per_second()) * dt;

		current_time = fmod(current_time, static_cast<float>(current_animation->get_duration()));
		calculate_bone_transform();
	}
}

void AnimationManager::calculate_bone_transform() {
	const Rig &rig = current_model->rig;
	std::vector<glm::mat4> global_transforms(rig.names.size());
	std::vector<glm::mat4> node_transforms(rig.names.size());

	for (int32_t i = 0; i < rig.names.size(); ++i) {
		std::string node_name = rig.names[i];
		node_transforms[i] = glm::translate(glm::mat4(1.0f), rig.positions[i]) * glm::toMat4(rig.rotations[i]);

		Channel *channel = current_animation->find_channel(node_name);
		if (channel) {
			channel->update(current_time);
			node_transforms[i] = channel->get_local_transform();
		}

		int64_t parent_index = rig.parents[i];
		if (parent_index >= 0) {
			global_transforms[i] = node_transforms[i] * global_transforms[parent_index];
		} else {
			global_transforms[i] = node_transforms[i];
		}
	}

	for (int32_t i = 0; i < rig.names.size(); ++i) {
		std::string node_name = rig.names[i];
		if (current_model->joint_map.find(node_name) != current_model->joint_map.end()) {
			int32_t index = current_model->joint_map[node_name].id;

			glm::mat4 offset = glm::translate(glm::mat4(1.0f), current_model->joint_map[node_name].translation) *
					glm::toMat4(current_model->joint_map[node_name].rotation);

			bone_matrices[index] = global_transforms[i] * offset;
		}
	}
}

void AnimationManager::change_animation(Animation *animation) {
	current_animation = animation;
	current_time = 0.0f;
}

const std::vector<glm::mat4> &AnimationManager::get_bone_matrices() const {
	return bone_matrices;
}

void AnimationManager::change_model(SkinnedModel *model) {
	current_model = model;
}
