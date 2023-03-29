#include "parent_manager.h"
#include "components/children_component.h"
#include "ecs_manager.h"

extern ECSManager ecs_manager;

bool ParentManager::add_children(Entity parent, Entity children) {
	if (!ecs_manager.has_component<Children>(parent)) {
		ecs_manager.add_component<Children>(parent, Children{});
	}

	return ecs_manager.get_component<Children>(parent).add_children(children);
}

bool ParentManager::remove_children(Entity parent, Entity children) {
	if (!ecs_manager.has_component<Children>(parent)) {
		return false;
	}

	if (ecs_manager.get_component<Children>(parent).children_count == 1) {
		ecs_manager.remove_component<Children>(parent);
		return true;
	}

	ecs_manager.get_component<Children>(parent).remove_children(children);
	return true;
}
