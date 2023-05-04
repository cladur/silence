#ifndef SILENCE_COLLIDER_COMPONENTS_FACTORY_H
#define SILENCE_COLLIDER_COMPONENTS_FACTORY_H

#include "components/collider_tag_component.h"
#include "components/static_tag_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "types.h"
#include <spdlog/spdlog.h>

class ColliderComponentsFactory {
public:
	template <typename T>
	static void add_collider_component(
			World &world, Entity entity, const T &collider_component, bool is_movable = false) {
		if (world.has_component<ColliderTag>(entity)) {
			SPDLOG_WARN("Failed to add collider component, object already has collider component");
			return;
		}
		if (!world.has_component<Transform>(entity)) {
			SPDLOG_WARN("Failed to add collider component, object has not transform component");
			return;
		}

		world.add_component<ColliderTag>(entity, {});
		if (!is_movable) {
			world.add_component<StaticTag>(entity, {});
		}
		world.add_component<T>(entity, collider_component);
	}

	template <typename T> static void remove_collider_component(World &world, Entity entity) {
		world.remove_component<ColliderTag>(entity);
		world.remove_component<T>(entity);
		if (world.has_component<StaticTag>(entity)) {
			world.remove_component<StaticTag>(entity);
		}
	}
};

#endif //SILENCE_COLLIDER_COMPONENTS_FACTORY_H
