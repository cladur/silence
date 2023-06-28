#include "ecs/systems/dialogue_system.h"
#include "agent_system.h"
#include "audio/event_reference.h"
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

void set_sentence_duration(Sentence &sentence) {
	FMOD::Studio::EventDescription *description;
	sentence.audio->getDescription(&description);
	int length = 0;
	description->getLength(&length);
	sentence.duration = length / 1000.0f;
}

#define SENTENCE(text1, text2, audio)                                                                                  \
	{ text1, text2, AudioManager::get().create_event_instance(audio), 1.0f }

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

	// TODO: Populate this with actual dialogue

	// Maksymalnie 60 znaków na tekst!!!

	// clang-format off
	dialogues["tut_intro"] = { {
		SENTENCE("Command: Hello, Hans and ScorpIO. This is your mission operator speaking.", "", "Dialogues/tut/tut_1"),
		SENTENCE("Command: A few days ago, we got reports of an unregulated", "construction of a combat robot by a military tech company Tech-Mil.", "Dialogues/tut/tut_2"),
		SENTENCE("Command: According to our sources, they've been", "working on it in secret for the last 3 years.", "Dialogues/tut/tut_3"),
		SENTENCE("Command: Your mission is to infiltrate their military base", "and find out how big of a threat is that robot.", "Dialogues/tut/tut_4"),
		SENTENCE("Command: It's just the two of you out there.", "We cannot provide you with any backup.", "Dialogues/tut/tut_5"),
		SENTENCE("Command: That's why it's crucial that you do not get caught.", "You need to stay out of sight of the cameras and enemies.", "Dialogues/tut/tut_6"),
	} };

	dialogues["tut_marking"] = { {
		SENTENCE("Command: You can hold the LEFT TRIGGER to zoom in.", "", "Dialogues/tut_mark/mark_1"),
		SENTENCE("Command: While zoomed in, focus on", "objects of interest in order to mark them.", "Dialogues/tut_mark/mark_2"),
		SENTENCE("Command: Try marking some enemies to the left of you.", "", "Dialogues/tut_mark/mark_3"),
		SENTENCE("Command: It's worth noting that", "marked enemies can be seen through walls.", "Dialogues/tut_mark/mark_4"),
		SENTENCE("Command: Once you're done, head to the next room.", "", "Dialogues/tut_mark/mark_5"),
	} };

	dialogues["tut_climbing_vent"] = { {
		SENTENCE("Hans: I think I can climb on those boxes.", "Do you want me to pick you up?", "Dialogues/climb/agent_climb"),
		SENTENCE("ScorpIO: No thanks, I can go through that vent.", "", "Dialogues/climb/hacker_climb"),
	} };

	dialogues["tut_camera"] = {{
		SENTENCE("Command: Look out! There's a security camera ahead of you.", "", "Dialogues/camera/camera_1"),
		SENTENCE("Command: The red light coming out of it means that it can spot you.", "", "Dialogues/camera/camera_2"),
		SENTENCE("ScorpIO: Don't worry, I can take control over it,", "letting both of us sneak past it.", "Dialogues/camera/camera_hacker"),
	}};

	dialogues["tut_enemies"] = {{
		SENTENCE("Command: The room ahead of you is full of enemies, watch out.", "", "Dialogues/enemies/enemy"),
		SENTENCE("ScorpIO: I'm afraid I don't have the abilities necessary", "for engaging in direct combat.", "Dialogues/enemies/enemy_hacker_1"),
		SENTENCE("Hans: I'll take them out, while you distract them, okay?", "", "Dialogues/enemies/enemy_agent"),
		SENTENCE("ScorpIO: Roger that.", "", "Dialogues/enemies/enemy_hacker_2"),
	}};


	//clang-format on
	for (auto &dialogue : dialogues) {
		for (auto &sentence : dialogue.second.sentences) {
			set_sentence_duration(sentence);
		}
	}
}

void DialogueSystem::play_dialogue() {
	dialogue_timer = 0.0f;
	current_dialogue_id = dialogue_queue.front();
	dialogue_queue.pop();
	current_sentence_id = 0;
}


void DialogueSystem::update(World &world, float dt) {
	ZoneScopedN("DialogueSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();

	if (first_frame) {
		first_frame = false;

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

	// Wait for global matrices of transforms to be updated
	if (world.get_parent_scene()->frame_number == 0) {
		return;
	}

	dialogue_timer += dt;

	if (!current_dialogue_id.empty()) {
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
			if (!dialogue_queue.empty()) {
				play_dialogue();
			} else {
				current_dialogue_id = "";
				current_sentence_id = 0;
				dialogue_timer = 0.0f;

				ui_dialogue_text->text = "";
				ui_dialogue_text2->text = "";
			}
		} else {
			auto &sentence = dialogue.sentences[current_sentence_id];

			if (!sentence.played) {
				sentence.played = true;
				SPDLOG_INFO("Playing audio {}", sentence.text);
				sentence.audio->start();
			}

			ui_dialogue_text->text = sentence.text;
			ui_dialogue_text2->text = sentence.text2;
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
			if (dialogue_trigger.dialogue_id.starts_with("level_")) {
				GameplayManager::get().change_scene(dialogue_trigger.dialogue_id);
				continue;
			}

			if (!dialogues.contains(dialogue_trigger.dialogue_id)) {
				SPDLOG_WARN("Dialogue {} does not exist", dialogue_trigger.dialogue_id);
				continue;
			}

			// Reset all sentences, in case the dialogue is triggered again (e.g. because we reloaded from checkpoint)
			Dialogue &dialogue = dialogues[dialogue_trigger.dialogue_id];
			for (auto &sentence : dialogue.sentences) {
				sentence.played = false;
			}

			SPDLOG_INFO("Dialogue triggered: {}", dialogue_trigger.dialogue_id);

			dialogue_trigger.triggered = true;

			dialogue_queue.push(dialogue_trigger.dialogue_id);

			if (dialogue_queue.size() == 1 && current_dialogue_id.empty()) {
				play_dialogue();
			}
		}
	}
}

DialogueSystem::~DialogueSystem() {
	UIManager::get().delete_ui_scene(ui_name);
}