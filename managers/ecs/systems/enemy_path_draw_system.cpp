#include "enemy_path_draw_system.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_path_component.h>

void EnemyPathDraw::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<PathParent>());
	whitelist.set(world.get_component_type<Children>());

	world.set_system_component_whitelist<EnemyPathDraw>(whitelist);
}

void EnemyPathDraw::update(World &world, float dt) {
	ZoneScopedN("EnemyPathDraw::update");
	auto &r_s = world.get_parent_scene()->get_render_scene();
	for (const Entity entity : entities) {
		auto &children = world.get_component<Children>(entity);

		glm::vec3 color;
		for (int i = 0; i < children.children_count; i++) {
			auto &path_node = world.get_component<PathNode>(children.children[i]);
			auto &t = world.get_component<Transform>(children.children[i]);

			int next_idx = (i + 1) % children.children_count;

			auto &t_next = world.get_component<Transform>(children.children[next_idx]);

			if (path_node.is_patrol_point) {
				color = glm::vec3(1.0f, 0.1f, 0.1f);
			} else {
				color = glm::vec3(1.0f, 0.8f, 0.0f);
			}
			r_s.debug_draw.draw_sphere(t.get_global_position(), 0.2f, color, entity);
			r_s.debug_draw.draw_line(
					t.get_global_position(), t_next.get_global_position(), glm::vec3(1.0f, 1.0f, 1.0f));
		}
	}
}
