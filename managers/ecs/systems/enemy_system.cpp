#include "enemy_system.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_data_component.h>
#include <components/enemy_path_component.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>

AutoCVarFloat cvar_enemy_detection_range("enemy.detection_range", "enemy detection range", 10.0f);
AutoCVarFloat cvar_enemy_detection_angle("enemy.detection_angle", "enemy detection angle", 90.0f);
AutoCVarFloat cvar_enemy_min_detection_speed("enemy.min_detection_speed", "enemy min detection speed", 0.1f);
AutoCVarFloat cvar_enemy_max_detection_speed("enemy.max_detection_speed", "enemy max detection speed", 0.5f);
AutoCVarFloat cvar_sphere_detection_radius("enemy.sphere_detection_radius", "enemy sphere detection radius", 2.5f);
AutoCVarFloat cvar_detection_decrease_speed("enemy.detection_decrease_speed", "enemy detection decrease speed", 0.3f);
AutoCVarFloat cvar_crouch_detection_modifier(
		"enemy.crouch_detection_modifier", "enemy crouch detection modifier", 0.5f);
AutoCVarFloat cvar_hacker_detection_modifier(
		"enemy.hacker_detection_modifier", "enemy hacker detection modifier", 0.75f);

void EnemySystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;
	whitelist.set(world.get_component_type<EnemyPath>());
	whitelist.set(world.get_component_type<EnemyData>());
	whitelist.set(world.get_component_type<AnimationInstance>());

	world.set_system_component_blacklist<EnemySystem>(blacklist);
	world.set_system_component_whitelist<EnemySystem>(whitelist);

	auto &rm = ResourceManager::get();
	rm.load_texture(asset_path("tag.ktx2").c_str());
}

void EnemySystem::update(World &world, float dt) {
	ZoneScopedN("EnemySystem::update");
	for (auto const &entity : entities) {
		auto &t = world.get_component<Transform>(entity);
		auto &ed = world.get_component<EnemyData>(entity);
		auto &ep = world.get_component<EnemyPath>(entity);
		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		if (ed.first_frame) { // i too hate having Start() function for a system / component 🥱🥱🥱🥱
			auto &ui = UIManager::get();
			auto &gm = GameplayManager::get();
			auto &rm = ResourceManager::get();
			gm.add_enemy_entity(entity);
			ed.state_machine.startup();
			ed.patrolling_state.startup(&ed.state_machine, "patrolling");
			ed.looking_state.startup(&ed.state_machine, "looking");
			ed.fully_aware_state.startup(&ed.state_machine, "fully_aware");
			ed.stationary_patrolling_state.startup(&ed.state_machine, "stationary_patrolling");
			ed.dead_state.startup(&ed.state_machine, "dying");
			ed.distracted_state.startup(&ed.state_machine, "distracted");

			ed.state_machine.add_state(&ed.patrolling_state);
			ed.state_machine.add_state(&ed.looking_state);
			ed.state_machine.add_state(&ed.fully_aware_state);
			ed.state_machine.add_state(&ed.stationary_patrolling_state);
			ed.state_machine.add_state(&ed.dead_state);
			ed.state_machine.add_state(&ed.distracted_state);

			if (ep.path_parent > 0) {
				if (world.has_component<Children>(ep.path_parent)) {
					auto &children = world.get_component<Children>(ep.path_parent);
					if (children.children_count > 1) {
						ed.state_machine.set_state("patrolling");
					} else {
						ed.state_machine.set_state("stationary_patrolling");
						ep.infinite_patrol = true;
					}
				} else {
					ed.state_machine.set_state("stationary_patrolling");
					ep.infinite_patrol = true;
				}
			} else {
				ed.state_machine.set_state("stationary_patrolling");
				ep.infinite_patrol = true;
			}

			ui.create_ui_scene(std::to_string(entity) + "_detection");
			ui.activate_ui_scene(std::to_string(entity) + "_detection");

			auto &agent_anchor = ui.add_ui_anchor(std::to_string(entity) + "_detection", "agent_anchor");
			agent_anchor.x = 0.25f;
			agent_anchor.y = 0.5f;
			agent_anchor.is_screen_space = true;
			agent_anchor.display = true;
			ui.add_as_root(std::to_string(entity) + "_detection", "agent_anchor");
			auto &hacker_anchor = ui.add_ui_anchor(std::to_string(entity) + "_detection", "hacker_anchor");
			hacker_anchor.x = 0.75f;
			hacker_anchor.y = 0.5f;
			hacker_anchor.is_screen_space = true;
			hacker_anchor.display = true;
			ui.add_as_root(std::to_string(entity) + "_detection", "hacker_anchor");

			auto &agent_detection_outline = ui.add_ui_image(std::to_string(entity) + "_detection",
					"agent_detection_outline");
			agent_detection_outline.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_outline.is_billboard = false;
			agent_detection_outline.is_screen_space = true;
			agent_detection_outline.size = glm::vec2(50.0f, 50.0f);
			agent_detection_outline.color = glm::vec4(1.0f);
			agent_detection_outline.texture = rm.load_texture(asset_path("detection_triangle_outline_white.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_outline", "agent_anchor");
			auto &hacker_detection_outline = ui.add_ui_image(std::to_string(entity) + "_detection",
					"hacker_detection_outline");
			hacker_detection_outline.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_outline.is_billboard = false;
			hacker_detection_outline.is_screen_space = true;
			hacker_detection_outline.size = glm::vec2(50.0f, 50.0f);
			hacker_detection_outline.color = glm::vec4(1.0f);
			hacker_detection_outline.texture = rm.load_texture(asset_path("detection_triangle_outline_white.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_outline", "hacker_anchor");
			auto &agent_detection_fill = ui.add_ui_image(std::to_string(entity) + "_detection",
					"agent_detection_fill");
			agent_detection_fill.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_fill.is_billboard = false;
			agent_detection_fill.is_screen_space = true;
			agent_detection_fill.size = glm::vec2(0.0f, 0.0f);
			agent_detection_fill.color = glm::vec4(1.0f);
			agent_detection_fill.texture = rm.load_texture(asset_path("detection_triangle_fill.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_fill", "agent_anchor");
			auto &hacker_detection_fill = ui.add_ui_image(std::to_string(entity) + "_detection",
					"hacker_detection_fill");
			hacker_detection_fill.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_fill.is_billboard = false;
			hacker_detection_fill.is_screen_space = true;
			hacker_detection_fill.size = glm::vec2(0.0f, 0.0f);
			hacker_detection_fill.color = glm::vec4(1.0f);
			hacker_detection_fill.texture = rm.load_texture(asset_path("detection_triangle_fill.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_fill", "hacker_anchor");

			auto &agent_detection_screen_flash = ui.add_ui_image(std::to_string(entity) + "_detection",
					"agent_detection_screen_flash");
			agent_detection_screen_flash.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_screen_flash.is_billboard = false;
			agent_detection_screen_flash.is_screen_space = true;
			agent_detection_screen_flash.size = glm::vec2(1.0f, 1.0f);
			agent_detection_screen_flash.color = glm::vec4(1.0f);
			agent_detection_screen_flash.texture = rm.load_texture(asset_path("detection_overlay.ktx2").c_str());
			agent_detection_screen_flash.display = false;
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_screen_flash", "agent_anchor");

			auto &hacker_detection_screen_flash = ui.add_ui_image(std::to_string(entity) + "_detection",
					"hacker_detection_screen_flash");
			hacker_detection_screen_flash.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_screen_flash.is_billboard = false;
			hacker_detection_screen_flash.is_screen_space = true;
			hacker_detection_screen_flash.size = glm::vec2(1.0f, 1.0f);
			hacker_detection_screen_flash.color = glm::vec4(1.0f);
			hacker_detection_screen_flash.texture = rm.load_texture(asset_path("detection_overlay.ktx2").c_str());
			hacker_detection_screen_flash.display = false;
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_screen_flash", "hacker_anchor");


			auto &tag = ui.add_ui_image(std::to_string(entity) + "_detection", "tag");
			tag.position = glm::vec3(0.0f, 1.6f, 0.0f);
			tag.size = glm::vec2(0.25f, 0.25f);
			tag.texture = rm.get_texture_handle("tag.ktx2");
			tag.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
			tag.display = false;
			ui.add_as_root(std::to_string(entity) + "_detection", "tag");

			ed.footsteps_event = EventReference("Agent/footstep");
			ed.death_event = EventReference("SFX/Enemies/death");

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