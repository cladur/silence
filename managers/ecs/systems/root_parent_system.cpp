#include "root_parent_system.h"
#include "components/children_component.h"
#include "components/transform_component.h"
#include "ecs/ecs_manager.h"
#include <components/parent_component.h>
extern ECSManager ecs_manager;

void RootParentSystem::startup() {
	Signature blacklist;
	Signature whitelist;
	whitelist.set(ecs_manager.get_component_type<Transform>());
	whitelist.set(ecs_manager.get_component_type<Children>());

	blacklist.set(ecs_manager.get_component_type<Parent>());
	ecs_manager.set_system_component_whitelist<RootParentSystem>(whitelist);
	ecs_manager.set_system_component_blacklist<RootParentSystem>(blacklist);
}

void RootParentSystem::update() {
	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		transform.update_global_model_matrix();
		auto model = transform.get_global_model_matrix();
		update_children(entity, model);
	}
}

void RootParentSystem::update_children(Entity parent, glm::mat4 parent_model) { // NOLINT(misc-no-recursion)
	auto children = ecs_manager.get_component<Children>(parent);

	for (int i = 0; i < children.children_count; i++) {
		Entity child = children.children[i];
		auto &child_transform = ecs_manager.get_component<Transform>(child);

		child_transform.update_global_model_matrix(parent_model);

		if (ecs_manager.has_component<Children>(child)) {
			update_children(child, child_transform.get_global_model_matrix());
		}
	}
}
