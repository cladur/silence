#include "collider_components_factory.h"
#include "components/collider_tag_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "spdlog/spdlog.h"

extern ECSManager ecs_manager;

template <typename T> void collider_components_factory::add_collider_component(Entity entity, T collider_component) {
	if (!ecs_manager.has_component<Transform>(entity)) {
		SPDLOG_WARN("Failed to add collider component, object has not transform component");
		return;
	}

	ecs_manager.add_component<ColliderTag>(entity, {});
	ecs_manager.add_component<T>(entity, collider_component);
}