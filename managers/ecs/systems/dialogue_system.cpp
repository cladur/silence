#include "ecs/systems/dialogue_system.h"
#include "agent_system.h"
#include "components/agent_data_component.h"
#include "components/collider_obb.h"
#include "components/static_tag_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "animation/animation_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <audio/audio_manager.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>

void DialogueSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<Name>());
	whitelist.set(world.get_component_type<ColliderOBB>());

	blacklist.set(world.get_component_type<StaticTag>());
	blacklist.set(world.get_component_type<ColliderTag>());

	world.set_system_component_whitelist<DialogueSystem>(whitelist);
	world.set_system_component_blacklist<DialogueSystem>(blacklist);

	auto &rm = ResourceManager::get();

	if (GameplayManager::get().first_start) {
		ui_name = "dialogue_ui";

		auto &ui = UIManager::get();
		ui.create_ui_scene(ui_name);
		ui.activate_ui_scene(ui_name);

		// anchor at the center of hacker's half of screen
		auto &root_anchor = ui.add_ui_anchor(ui_name, "root_anchor");
		root_anchor.is_screen_space = true;
		root_anchor.x = 0.5f;
		root_anchor.y = 0.2f;
		root_anchor.display = true;
		ui.add_as_root(ui_name, "root_anchor");

		ui_dialogue_text = &ui.add_ui_text(ui_name, "dialogue_text");
		ui_dialogue_text->text = "";
		ui_dialogue_text->is_screen_space = true;
		ui_dialogue_text->size = glm::vec2(0.75f);
		ui_dialogue_text->position = glm::vec3(0.0f, 0.0f, 0.0f);
		ui_dialogue_text->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.9f);
		ui_dialogue_text->centered_y = true;
		ui_dialogue_text->centered_x = true;

		ui.add_to_root(ui_name, "dialogue_text", "root_anchor");

		ui_dialogue_text2 = &ui.add_ui_text(ui_name, "dialogue_text2");
		ui_dialogue_text2->text = "";
		ui_dialogue_text2->is_screen_space = true;
		ui_dialogue_text2->size = glm::vec2(0.75f);
		ui_dialogue_text2->position = glm::vec3(0.0f, -48.0f, 0.0f);
		ui_dialogue_text2->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.9f);
		ui_dialogue_text2->centered_y = true;
		ui_dialogue_text2->centered_x = true;

		ui.add_to_root(ui_name, "dialogue_text2", "root_anchor");
	}
}

void DialogueSystem::update(World &world, float dt) {
	ZoneScopedN("DialogueSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();

	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &name = world.get_component<Name>(entity);
		auto &collider = world.get_component<ColliderOBB>(entity);

		ColliderOBB c = {};

		const glm::quat &orientation = transform.get_global_orientation();
		c.set_orientation(orientation);
		c.range = collider.range * transform.get_global_scale();
		c.center = transform.get_global_position() + orientation * (collider.center * transform.get_global_scale());

		auto vec = PhysicsManager::get().overlap_cube_checkpoint(world, c);

		SPDLOG_INFO("{}: {}", name.name, vec.size());
		if (!vec.empty()) {
			ui_dialogue_text->text = "Example dialogue text here,";
			ui_dialogue_text2->text = "lorem epsum dolor sit amet.";
		}
	}
}

DialogueSystem::~DialogueSystem() {
	UIManager::get().delete_ui_scene(ui_name);
}