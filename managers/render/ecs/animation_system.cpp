#include "animation_system.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "managers/animation/joint.h"
#include "managers/render/common/animation.h"
#include "managers/render/render_manager.h"
#include "render/common/skinned_model.h"
#include "resource/resource_manager.h"
#include <glm/gtx/quaternion.hpp>

void AnimationSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<AnimationInstance>());
	signature.set(world.get_component_type<SkinnedModelInstance>());
	world.set_system_component_whitelist<AnimationSystem>(signature);
}

void AnimationSystem::update(World &world, float dt) {
	for (Entity entity : entities) {
		if (animation_map.find(entity) == animation_map.end()) {
			animation_map[entity].model = &world.get_component<SkinnedModelInstance>(entity);
			animation_map[entity].animation = &world.get_component<AnimationInstance>(entity);
			animation_map[entity].model->bone_matrices = std::vector<glm::mat4>(MAX_BONE_COUNT, glm::mat4(1.0f));
		}
		update_animation(animation_map[entity], dt);
	}
}

void AnimationSystem::update_animation(AnimData &data, float dt) {
	if (data.animation) {
		ResourceManager &resource_manager = ResourceManager::get();
		Animation &animation = resource_manager.get_animation(data.animation->animation_handle);
		float &current_time = data.animation->current_time;
		current_time += static_cast<float>(animation.get_ticks_per_second()) * dt;

		if (current_time < animation.get_duration() || data.animation->is_looping) {
			current_time = fmod(current_time, static_cast<float>(animation.get_duration()));
			calculate_bone_transform(data);
		}
	}
}

void AnimationSystem::calculate_bone_transform(AnimData &data) {
	ResourceManager &resource_manager = ResourceManager::get();
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);
	Animation &animation = resource_manager.get_animation(data.animation->animation_handle);

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

			data.model->bone_matrices[index] = global_matrices[i] * offset;
		}
	}
}
