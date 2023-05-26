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
			ed.dead_state.startup(&ed.state_machine, "dying");

			ed.state_machine.add_state(&ed.patrolling_state);
			ed.state_machine.add_state(&ed.looking_state);
			ed.state_machine.add_state(&ed.fully_aware_state);
			ed.state_machine.add_state(&ed.stationary_patrolling_state);
			ed.state_machine.add_state(&ed.dead_state);

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

		// whole logic is happening in the state machine's states
		ed.state_machine.update(&world, entity, dt);
	}
}

void EnemySystem::death(World &world, uint32_t entity) {
	auto &ed = world.get_component<EnemyData>(entity);
	ed.state_machine.set_state("dead");
}