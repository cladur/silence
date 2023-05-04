#include "isolated_entities_system.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include <components/children_component.h>
#include <components/parent_component.h>

void IsolatedEntitiesSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;
	blacklist.set(world.get_component_type<Children>());
	blacklist.set(world.get_component_type<Parent>());
	whitelist.set(world.get_component_type<Transform>());

	world.set_system_component_blacklist<IsolatedEntitiesSystem>(blacklist);
	world.set_system_component_whitelist<IsolatedEntitiesSystem>(whitelist);
}
void IsolatedEntitiesSystem::update(World &world, float dt) {
	for (auto const &entity : entities) {
		world.get_component<Transform>(entity).update_global_model_matrix();
	}
}
