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

struct Sentence {
	std::string text;
	std::string audio;
	float duration;
};

struct Dialogue {
	std::vector<Sentence> sentences;
};

float dialogue_timer = 0.0f;
int current_dialogue_id = -1;
int current_sentence_id = 0;

// TODO: Populate this with actual dialogue
static std::unordered_map<int, Dialogue> dialogues = {
	{ 0,
			{ { { "DIALOGUE 0 - Hello, I am the hacker. I will be your guide.", "hacker_intro", 3.0f },
					{ "You are in a simulation. You must escape.", "hacker_intro2", 3.0f },
					{ "You must find the exit.", "hacker_intro3", 3.0f }, { "Good luck.", "hacker_intro4", 3.0f } } } },
	{ 1,
			{ { { "DIALOGUE 1 - Hello, I am the hacker. I will be your guide.", "hacker_intro", 3.0f },
					{ "You are in a simulation. You must escape.", "hacker_intro2", 3.0f },
					{ "You must find the exit.", "hacker_intro3", 3.0f }, { "Good luck.", "hacker_intro4", 3.0f } } } },
	{ 2,
			{ { { "DIALOGUE 2 - Hello, I am the hacker. I will be your guide.", "hacker_intro", 3.0f },
					{ "You are in a simulation. You must escape.", "hacker_intro2", 3.0f },
					{ "You must find the exit.", "hacker_intro3", 3.0f }, { "Good luck.", "hacker_intro4", 3.0f } } } },
};

void DialogueSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<DialogueTrigger>());
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

	// Wait for global matrices of transforms to be updated
	if (world.get_parent_scene()->frame_number == 0) {
		return;
	}

	dialogue_timer += dt;

	if (current_dialogue_id != -1) {
		auto &dialogue = dialogues[current_dialogue_id];

		for (int i = current_sentence_id; i < dialogue.sentences.size(); i++) {
			auto &sentence = dialogue.sentences[i];
			if (dialogue_timer < sentence.duration) {
				break;
			}
			current_sentence_id++;
			dialogue_timer -= sentence.duration;
		}

		if (current_sentence_id >= dialogue.sentences.size()) {
			current_dialogue_id = -1;
			current_sentence_id = 0;
			dialogue_timer = 0.0f;

			ui_dialogue_text->text = "";
			ui_dialogue_text2->text = "";
		} else {
			auto &sentence = dialogue.sentences[current_sentence_id];

			ui_dialogue_text->text = sentence.text;
		}
	}

	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &dialogue_trigger = world.get_component<DialogueTrigger>(entity);
		auto &collider = world.get_component<ColliderOBB>(entity);

		if (dialogue_trigger.triggered) {
			continue;
		}

		ColliderOBB c = {};

		const glm::quat &orientation = transform.get_global_orientation();
		c.set_orientation(orientation);
		c.range = collider.range * transform.get_global_scale();
		c.center = transform.get_global_position() + orientation * (collider.center * transform.get_global_scale());

		auto vec = PhysicsManager::get().overlap_cube_checkpoint(world, c);

		if (!vec.empty()) {
			if (!dialogues.contains(dialogue_trigger.dialogue_id)) {
				SPDLOG_WARN("Dialogue {} does not exist", dialogue_trigger.dialogue_id);
				continue;
			}

			SPDLOG_INFO("Dialogue triggered: {}", dialogue_trigger.dialogue_id);

			dialogue_trigger.triggered = true;
			dialogue_timer = 0.0f;
			current_dialogue_id = dialogue_trigger.dialogue_id;
			current_sentence_id = 0;
		}
	}
}

DialogueSystem::~DialogueSystem() {
	UIManager::get().delete_ui_scene(ui_name);
}