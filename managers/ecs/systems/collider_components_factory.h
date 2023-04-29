#ifndef SILENCE_COLLIDER_COMPONENTS_FACTORY_H
#define SILENCE_COLLIDER_COMPONENTS_FACTORY_H

#include "components/collider_tag_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "types.h"
#include <spdlog/spdlog.h>

class ColliderComponentsFactory {
public:
	template <typename T> static void add_collider_component(Entity entity, const T &collider_component) {
		ECSManager &ecs_manager = ECSManager::get();

		if (ecs_manager.has_component<ColliderTag>(entity)) {
			SPDLOG_WARN("Failed to add collider component, object already has collider component");
			return;
		}
		if (!ecs_manager.has_component<Transform>(entity)) {
			SPDLOG_WARN("Failed to add collider component, object has not transform component");
			return;
		}

		ecs_manager.add_component<ColliderTag>(entity, {});
		ecs_manager.add_component<T>(entity, collider_component);
	}

	template <typename T> static void remove_collider_component(Entity entity) {
		ECSManager &ecs_manager = ECSManager::get();
		ecs_manager.remove_component<ColliderTag>(entity);
		ecs_manager.remove_component<T>(entity);
	}
};

#endif //SILENCE_COLLIDER_COMPONENTS_FACTORY_H
