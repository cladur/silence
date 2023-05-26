#include "enemy_system.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_data_component.h>
#include <components/enemy_path_component.h>
#include <render/transparent_elements/ui_manager.h>

void EnemySystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;
	whitelist.set(world.get_component_type<EnemyPath>());
	whitelist.set(world.get_component_type<EnemyData>());
	whitelist.set(world.get_component_type<AnimationInstance>());

	world.set_system_component_blacklist<EnemySystem>(blacklist);
	world.set_system_component_whitelist<EnemySystem>(whitelist);
}

void EnemySystem::update(World &world, float dt) {
	for (auto const &entity : entities) {
		auto &t = world.get_component<Transform>(entity);
		auto &ed = world.get_component<EnemyData>(entity);
		auto &ep = world.get_component<EnemyPath>(entity);
		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		if (ed.first_frame) { // i too hate having Start() function for a system / component 🥱🥱🥱🥱
			auto &ui = UIManager::get();
			ed.state_machine.startup();
			ed.patrolling_state.startup(&ed.state_machine, "patrolling");
			ed.looking_state.startup(&ed.state_machine, "looking");
			ed.fully_aware_state.startup(&ed.state_machine, "fully_aware");
			ed.stationary_patrolling_state.startup(&ed.state_machine, "stationary_patrolling");
			ed.state_machine.add_state(&ed.patrolling_state);
			ed.state_machine.add_state(&ed.looking_state);
			ed.state_machine.add_state(&ed.fully_aware_state);
			ed.state_machine.add_state(&ed.stationary_patrolling_state);
			ed.state_machine.set_state("patrolling");

			ui.create_ui_scene(std::to_string(entity) + "_detection");
			auto &slider = ui.add_ui_slider(
					std::to_string(entity) + "_detection",
					"detection_slider");
			slider.position = glm::vec3(0.0f, 2.0f, 0.0f);
			slider.is_billboard = true;
			slider.is_screen_space = false;
			slider.size = glm::vec2(0.1f, 0.5f);
			slider.slider_alignment = SliderAlignment::BOTTOM_TO_TOP;
			slider.color = glm::vec3(1.0f);

			ui.add_ui_slider(std::to_string(entity) + "_detection", "detection_slider");
			ui.add_as_root(std::to_string(entity) + "_detection", "detection_slider");
			ui.activate_ui_scene(std::to_string(entity) + "_detection");
			ed.first_frame = false;
		}

		ed.state_machine.update(&world, entity, dt);

		// draw a flat triangle representing vision using draw_line
//		glm::vec4 vision_start = glm::vec4(t.position + glm::vec3(0.0f, 0.1f, 0.0f), 1.0f);
//		glm::vec4 forward = glm::vec4(t.get_global_forward(), 1.0f);
//		glm::mat4 rot = glm::rotate(
//				glm::mat4(1.0f),
//				glm::radians(ed.view_cone_angle / 2.0f),
//				glm::vec3(0.0f, 1.0f, 0.0f));
//		glm::vec3 vision_end_left = forward * rot;
//		rot = glm::rotate(
//				glm::mat4(1.0f),
//				glm::radians(-ed.view_cone_angle / 2.0f),
//				glm::vec3(0.0f, 1.0f, 0.0f));
//		glm::vec3 vision_end_right = forward * rot;
//
//		dd.draw_line(
//				vision_start,
//				vision_start + glm::vec4(glm::normalize(vision_end_left), 1.0f) * ed.view_cone_distance,
//				glm::vec3(1.0f, 0.0f, 0.0f));
//		dd.draw_line(
//				vision_start,
//				vision_start + glm::vec4(glm::normalize(vision_end_right), 1.0f) * ed.view_cone_distance,
//				glm::vec3(1.0f, 0.0f, 0.0f));
//
//		dd.draw_line(
//				vision_start + glm::vec4(glm::normalize(vision_end_left), 1.0f) * ed.view_cone_distance,
//				vision_start + forward * ed.view_cone_distance,
//				glm::vec3(1.0f, 0.0f, 0.0f));
//		dd.draw_line(
//				vision_start + glm::vec4(glm::normalize(vision_end_right), 1.0f) * ed.view_cone_distance,
//				vision_start + forward * ed.view_cone_distance,
//				glm::vec3(1.0f, 0.0f, 0.0f));
	}
}
