#include "enemy_path_draw_system.h"
#include <components/enemy_path_component.h>
#include "ecs/world.h"
#include "engine/scene.h"

void EnemyPathDraw::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<EnemyPath>());

	world.set_system_component_whitelist<EnemyPathDraw>(whitelist);
}

void EnemyPathDraw::update(World &world, float dt) {
	auto &r_s = world.get_parent_scene()->get_render_scene();
	for (const Entity entity : entities) {
		auto &enemy_path = world.get_component<EnemyPath>(entity);

		int size = enemy_path.path.size();
		glm::vec3 color;
		for (int i = 0; i < size; i++) {
			if (enemy_path.patrol_points[i].second) {
				color = glm::vec3(1.0f, 0.1f, 0.1f);
			}
			else {
				color = glm::vec3(1.0f, 0.8f, 0.0f);
			}
			r_s.debug_draw.draw_sphere(enemy_path.path[i], 0.2f, color);
			r_s.debug_draw.draw_line(
					enemy_path.path[i],
					enemy_path.path[(i + 1) % enemy_path.path.size()],
					glm::vec3(1.0f, 1.0f, 1.0f));
		}

	}
}
