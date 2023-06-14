#include "cable_system.h"
#include "ecs/world.h"
#include "engine/scene.h"

void CableSystem::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<CableParent>());
	whitelist.set(world.get_component_type<Children>());

	world.set_system_component_whitelist<CableSystem>(whitelist);
}

void CableSystem::update(World &world, float dt) {
	ZoneScopedN("CableSystem::update");
	for (const Entity entity : entities) {
		auto &cable = world.get_component<CableParent>(entity);
		auto &children = world.get_component<Children>(entity);

		if (children.children_count > 0) {
			for (auto &child : children.children) {
				if (world.has_component<Highlight>(child)) {
					auto &highlight = world.get_component<Highlight>(child);

					highlight.highlighted = true;
					highlight.highlight_color = (cable.state == CableState::ON) ? cable.on_color : cable.off_color;

					if (cable.state == CableState::OFF && !cable.highlighted_on_off) {
						highlight.highlighted = false;
					}
				}
			}
		}
	}
}
