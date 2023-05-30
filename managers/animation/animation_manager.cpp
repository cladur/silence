#include "animation_manager.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "render/common/animation.h"
#include "render/common/skinned_model.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include <glm/gtx/quaternion.hpp>

AutoCVarFloat cvar_blend_time_ms("animation.blend_time", "blend time in ms", 700.0f, CVarFlags::EditFloatDrag);

AnimationManager &AnimationManager::get() {
	static AnimationManager instance;
	return instance;
}

void AnimationManager::update_pose(AnimData &data, float dt) {
	if (!data.animation) {
		SPDLOG_WARN("Data animation is not set.");
		assert(false);
		return;
	}

	if (data.animation->is_freeze) {
		return;
	}
	ResourceManager &resource_manager = ResourceManager::get();
	Animation &animation = resource_manager.get_animation(data.animation->animation_handle);
	float &current_time = data.animation->current_time;
	current_time += data.animation->ticks_per_second * dt;

	if (current_time >= animation.get_duration() && !data.animation->is_looping) {
		return;
	}

	if (data.has_changed && cvar_blend_time_ms.get() > 0.0f) {
		Animation &next_animation = resource_manager.get_animation(data.animation->animation_handle);
		current_time = fmod(current_time, next_animation.get_duration());
		Pose next_pose;
		next_animation.sample(data, next_pose);

		float alpha = current_time / cvar_blend_time_ms.get();
		blend_poses(data.local_pose, next_pose, data.local_pose, std::min(calculate_uniform_s(alpha), 1.0f));

		if (alpha >= 1.0f) {
			data.has_changed = false;
		}
	} else {
		current_time = fmod(current_time, animation.get_duration());
		animation.sample(data, data.local_pose);
	}
	local_to_model(data);
	model_to_final(data);
}

void AnimationManager::local_to_model(AnimData &data) {
	ResourceManager &resource_manager = ResourceManager::get();
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);

	Rig &rig = model.rig;

	std::vector<Xform> &local_matrices = data.local_pose.xforms;
	data.model_pose.xforms.resize(data.local_pose.xforms.size());
	std::vector<Xform> &global_matrices = data.model_pose.xforms;

	for (int32_t i = 0; i < local_matrices.size(); ++i) {
		int32_t parent_index = rig.parents[i];
		if (parent_index != -1) {
			global_matrices[i] = global_matrices[parent_index] * local_matrices[i];
		} else {
			global_matrices[i] = local_matrices[i];
		}
	}
}

void AnimationManager::model_to_final(AnimData &data) {
	ResourceManager &resource_manager = ResourceManager::get();
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);

	Rig &rig = model.rig;
	if (data.model->bone_matrices.size() < MAX_BONE_COUNT) {
		data.model->bone_matrices.resize(MAX_BONE_COUNT);
	}

	std::vector<Xform> &global_matrices = data.model_pose.xforms;

	for (int32_t i = 0; i < global_matrices.size(); ++i) {
		const auto &joint = model.joint_map.find(rig.names[i]);
		if (joint != model.joint_map.end()) {
			int32_t index = joint->second.id;

			data.model->bone_matrices[index] =
					glm::mat4(global_matrices[i] * Xform(joint->second.translation, joint->second.rotation));
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
	entity_data.animation->animation_handle = resource_manager.get_animation_handle(new_animation_name);
	entity_data.animation->current_time = 0.0f;
	entity_data.has_changed = true;
	entity_data.animation->is_looping = true;
}

void AnimationManager::blend_poses(const Pose &pose1, const Pose &pose2, Pose &result_pose, float alpha) {
	if (pose1.xforms.size() != pose2.xforms.size() || pose1.xforms.size() != result_pose.xforms.size()) {
		SPDLOG_WARN("Poses have not equal xforms count {} {} {}", pose1.xforms.size(), pose2.xforms.size(),
				result_pose.xforms.size());
		assert(false);
		return;
	}

	for (int32_t i = 0; i < result_pose.xforms.size(); ++i) {
		slerp(pose1.xforms[i], pose2.xforms[i], result_pose.xforms[i], alpha);
	}
}

void AnimationManager::slerp(const Xform &m1, const Xform &m2, Xform &result, float alpha) {
	result.rotation = glm::normalize(glm::slerp(m1.rotation, m2.rotation, alpha));
	result.translation = glm::mix(m1.translation, m2.translation, alpha);
}

glm::mat4 AnimationManager::get_bone_transform(Entity holder, std::string &bone_name) {
	ResourceManager &resource_manager = ResourceManager::get();
	AnimData &data = animation_map[holder];
	SkinnedModel &model = resource_manager.get_skinned_model(data.model->model_handle);

	for (int32_t i = 0; i < data.model_pose.xforms.size(); ++i) {
		if (model.rig.names[i] == bone_name) {
			return glm::mat4(data.model_pose.xforms[i]);
		}
	}
	SPDLOG_WARN("Bone with name {} not found in model {}.", bone_name, model.name);
	bone_name = "";
	return glm::mat4(1.0f);
}

void AnimationManager::attach_to_entity(World &world, Entity holder, Entity attached, const std::string &bone_name) {
	if (!world.has_component<SkinnedModelInstance>(holder)) {
		SPDLOG_WARN("Cannot attach to {}, entity has not have SkinnedModelInstance.", holder);
		assert(false);
		return;
	}

	Attachment component{ bone_name, holder };
	world.add_component(attached, component);
}

void AnimationManager::detach_entity(World &world, Entity attached) {
	if (!world.has_component<Attachment>(attached)) {
		SPDLOG_WARN("Cannot detach {}, entity has not have Attachment component.", attached);
		assert(false);
		return;
	}

	world.remove_component<Attachment>(attached);
}
