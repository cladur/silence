#include "animation_system.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "managers/animation/joint.h"
#include "managers/render/common/animation.h"
#include "managers/render/render_manager.h"
#include "render/common/skinned_model.h"
#include <glm/gtx/quaternion.hpp>

void AnimationSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<ModelInstance>());
	world.set_system_component_whitelist<AnimationSystem>(signature);
}

void AnimationSystem::update(World &world, float dt) {
	update_animation(dt);
}

AnimationSystem::AnimationSystem(Animation &animation, SkinnedModel &model) {
	current_time = 0.0f;
	current_animation = &animation;
	current_model = &model;

	bone_matrices.reserve(MAX_BONE_COUNT);

	for (int32_t i = 0; i < MAX_BONE_COUNT; ++i) {
		bone_matrices.emplace_back(1.0f);
	}
}

void AnimationSystem::update_animation(float dt) {
	delta_time = dt;
	if (current_animation) {
		current_time += static_cast<float>(current_animation->get_ticks_per_second()) * dt;

		current_time = fmod(current_time, static_cast<float>(current_animation->get_duration()));
		calculate_bone_transform();
	}
}

void AnimationSystem::calculate_bone_transform() {
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

void AnimationSystem::change_animation(Animation *animation) {
	current_animation = animation;
	current_time = 0.0f;
}

const std::vector<glm::mat4> &AnimationSystem::get_bone_matrices() const {
	return bone_matrices;
}

void AnimationSystem::change_model(SkinnedModel *model) {
	current_model = model;
}
