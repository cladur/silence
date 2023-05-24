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
		auto &ed = world.get_component<EnemyData>(entity);
		auto &ep = world.get_component<EnemyPath>(entity);

		if (ed.first_frame) { // i too hate having Start() function for a system / component 🥱🥱🥱🥱
			auto &ui = UIManager::get();
			ed.state_machine.startup();
			ed.patrolling_state.startup(&ed.state_machine, "patrolling");
			ed.looking_state.startup(&ed.state_machine, "looking");
			ed.state_machine.add_state(&ed.patrolling_state);
			ed.state_machine.add_state(&ed.looking_state);
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
	}
}
