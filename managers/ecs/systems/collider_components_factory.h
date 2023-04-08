#ifndef SILENCE_COLLIDER_COMPONENTS_FACTORY_H
#define SILENCE_COLLIDER_COMPONENTS_FACTORY_H

#include "components/collider_tag_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "physics_system.h"
#include "types.h"
#include <spdlog/spdlog.h>

extern ECSManager ecs_manager;
extern PhysicsSystem physics_system;

class ColliderComponentsFactory {
public:
	template <typename T> static void add_collider_component(Entity entity, const T &collider_component) {
		if (!ecs_manager.has_component<Transform>(entity)) {
			SPDLOG_WARN("Failed to add collider component, object has not transform component");
			return;
		}

		ecs_manager.add_component<ColliderTag>(entity, {});
		ecs_manager.add_component<T>(entity, collider_component);
		physics_system.entities_with_collider.push_back(entity);
	}

	template <typename T> static void remove_collider_component(Entity entity, const T &collider_component) {
		ecs_manager.remove_component<ColliderTag>(entity);
		ecs_manager.remove_component<T>(entity);
		physics_system.entities_with_collider.remove(entity);
	}
};

#endif //SILENCE_COLLIDER_COMPONENTS_FACTORY_H
