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
	signature.set(world.get_component_type<AnimationInstance>());
	signature.set(world.get_component_type<SkinnedModelInstance>());
	world.set_system_component_whitelist<AnimationSystem>(signature);
}

void AnimationSystem::update(World &world, float dt) {
	RenderManager &render_manager = RenderManager::get();
	for (Entity entity : entities) {
		if (animation_map.find(entity) == animation_map.end()) {
			animation_map[entity].model = &world.get_component<SkinnedModelInstance>(entity);
			animation_map[entity].animation = &world.get_component<AnimationInstance>(entity);
			animation_map[entity].current_time = 0.0f;
			animation_map[entity].model->bone_matrices.reserve(MAX_BONE_COUNT);
			for (int32_t i = 0; i < MAX_BONE_COUNT; ++i) {
				animation_map[entity].model->bone_matrices.emplace_back(1.0f);
			}
			animation_map[entity].model->bone_matrices.reserve(MAX_BONE_COUNT);
		}
		update_animation(animation_map[entity], dt);
	}
}

void AnimationSystem::update_animation(AnimData &data, float dt) {
	if (data.animation) {
		RenderManager &render_manager = RenderManager::get();
		SkinnedModel &model = render_manager.get_skinned_model(data.model->model_handle);
		Animation &animation = render_manager.get_animation(data.animation->animation_handle);
		data.current_time += static_cast<float>(animation.get_ticks_per_second()) * dt;

		data.current_time = fmod(data.current_time, static_cast<float>(animation.get_duration()));
		calculate_bone_transform(data, model.root, glm::mat4(1.0f));
	}
}

void AnimationSystem::calculate_bone_transform(AnimData &data, const Bone &bone, const glm::mat4 &parent_transform) {
	RenderManager &render_manager = RenderManager::get();
	SkinnedModel &model = render_manager.get_skinned_model(data.model->model_handle);
	Animation &animation = render_manager.get_animation(data.animation->animation_handle);

	std::string node_name = bone.name;

	glm::mat4 node_transform = glm::translate(glm::mat4(1.0f), bone.translation) * glm::toMat4(bone.rotation);

	Channel *channel = animation.find_channel(node_name);

	if (channel) {
		channel->update(data.current_time);
		node_transform = channel->get_local_transform();
	}

	glm::mat4 global_transform = parent_transform * node_transform;

	if (model.joint_map.find(node_name) != model.joint_map.end()) {
		int32_t joint = model.joint_map[node_name].id;

		glm::mat4 offset = glm::translate(glm::mat4(1.0f), model.joint_map[node_name].translation) *
				glm::toMat4(model.joint_map[node_name].rotation);

		data.model->bone_matrices[joint] = global_transform * offset;
	}

	for (auto &child : bone.children) {
		calculate_bone_transform(data, child, global_transform);
	}
}