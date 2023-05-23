#include "animation_system.h"
#include "animation/animation_manager.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"

void AnimationSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<AnimationInstance>());
	signature.set(world.get_component_type<SkinnedModelInstance>());
	world.set_system_component_whitelist<AnimationSystem>(signature);
}

void AnimationSystem::update(World &world, float dt) {
	auto &animation_manager = AnimationManager::get();
	auto &resource_manager = ResourceManager::get();
	auto &animation_map = animation_manager.animation_map;
	for (Entity entity : entities) {
		if (!animation_map.contains(entity)) {
			AnimData &data = animation_map[entity];
			data.model = &world.get_component<SkinnedModelInstance>(entity);
			size_t size = resource_manager.get_skinned_model(data.model->model_handle).rig.names.size();
			data.animation = &world.get_component<AnimationInstance>(entity);
			data.local_pose.xforms = std::vector<Xform>(size, Xform{ glm::vec3(1.0f), glm::quat() });
			data.model_pose.xforms = std::vector<Xform>(size, Xform{ glm::vec3(1.0f), glm::quat() });
			data.model->bone_matrices = std::vector<glm::mat4>(animation_manager.MAX_BONE_COUNT, glm::mat4(1.0f));
		}
		animation_manager.update_pose(animation_map[entity], dt);
	}
}
