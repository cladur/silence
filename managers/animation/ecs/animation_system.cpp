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
	auto &animation_map = animation_manager.animation_map;
	for (Entity entity : entities) {
		if (animation_map.find(entity) == animation_map.end()) {
			animation_map[entity].model = &world.get_component<SkinnedModelInstance>(entity);
			animation_map[entity].animation = &world.get_component<AnimationInstance>(entity);
			animation_map[entity].model->bone_matrices =
					std::vector<glm::mat4>(animation_manager.MAX_BONE_COUNT, glm::mat4(1.0f));
		}
		animation_manager.update_pose(animation_map[entity], dt);
	}
}
