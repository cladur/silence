#include "show_taggable_system.h"
#include "engine/scene.h"

void ShowTaggable::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<Taggable>());
	world.set_system_component_whitelist<ShowTaggable>(whitelist);
}

void ShowTaggable::update(World &world, float dt) {
	auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

	for (auto const &entity : entities) {
		auto &taggable = world.get_component<Taggable>(entity);
		auto &transform = world.get_component<Transform>(entity);
		dd.draw_sphere(transform.get_global_position() + taggable.tag_position, 0.2f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	}
}
