#include "root_parent_system.h"
#include "components/children_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include <components/parent_component.h>

void RootParentSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;
	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<Children>());

	blacklist.set(world.get_component_type<Parent>());

	world.set_system_component_whitelist<RootParentSystem>(whitelist);
	world.set_system_component_blacklist<RootParentSystem>(blacklist);
}

void RootParentSystem::update(World &world, float dt) {
	ZoneScopedN("RootParentSystem::update");
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		transform.update_global_model_matrix();
		auto model = transform.get_global_model_matrix();
		update_children(world, entity, model);
	}
}

void RootParentSystem::update_children(
		World &world, Entity parent, const glm::mat4 &parent_model) { // NOLINT(misc-no-recursion)
	auto children = world.get_component<Children>(parent);

	for (int i = 0; i < children.children_count; i++) {
		Entity child = children.children[i];

		// Skip children that are attached to a bone, AttachmentSystem handles those
		if (world.has_component<Attachment>(child)) {
			continue;
		}

		auto &child_transform = world.get_component<Transform>(child);

		child_transform.update_global_model_matrix(parent_model);

		if (world.has_component<Children>(child)) {
			update_children(world, child, child_transform.get_global_model_matrix());
		}
	}
}
