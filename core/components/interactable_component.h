#ifndef SILENCE_INTERACTABLE_COMPONENT_H
#define SILENCE_INTERACTABLE_COMPONENT_H

#include <array>
enum class InteractionType { None, Agent, Hacker };
enum class Interaction {
	NoInteraction,
	HackerCameraJump,
	HackerPlatform,
	Exploding,
	LightSwitch,
	TemporalLightSwitch,
	SwitchMainDoorLight
};

struct Interactable {
	InteractionType type = InteractionType::None;
	Interaction interaction = Interaction::NoInteraction;

	std::array<Entity, 5> interaction_targets = {};
	Entity cable_parent = 0;
	Entity lever = 0;

	Entity enemy_entity = 0;
	Entity enemy_entity2 = 0;

	Entity main_door = 0;

	bool triggered = false;
	bool can_interact = true;
	bool single_use = false;
	bool is_powering_up = false;

	bool is_on = false;
	bool is_rotating = false;

	float temporal_switch_time = 0.0f;

	std::string interaction_text;

	bool first_frame = true;

	std::string ui_name = "ui_interaction_scene";

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["type"] = type;
		serialized_component["interaction"] = interaction;
		serialized_component["can_interact"] = can_interact;
		serialized_component["interaction_target"] = interaction_targets[0];
		serialized_component["interaction_target_2"] = interaction_targets[1];
		serialized_component["interaction_target_3"] = interaction_targets[2];
		serialized_component["interaction_target_4"] = interaction_targets[3];
		serialized_component["interaction_target_5"] = interaction_targets[4];
		serialized_component["enemy_entity"] = enemy_entity;
		serialized_component["enemy_entity2"] = enemy_entity2;

		serialized_component["single_use"] = single_use;
		serialized_component["is_on"] = is_on;

		serialized_component["interaction_text"] = interaction_text;

		serialized_component["cable_parent"] = cable_parent;
		serialized_component["lever"] = lever;
		serialized_component["main_door"] = main_door;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Interactable";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		type = serialized_component["type"];
		interaction = serialized_component["interaction"];
		can_interact = serialized_component["can_interact"];
		if (serialized_component.contains("interaction_target")) {
			interaction_targets[0] = serialized_component["interaction_target"];
		} else {
			interaction_targets[0] = 0;
		}

		if (serialized_component.contains("interaction_target_2")) {
			interaction_targets[1] = serialized_component["interaction_target_2"];
		} else {
			interaction_targets[1] = 0;
		}

		if (serialized_component.contains("interaction_target_3")) {
			interaction_targets[2] = serialized_component["interaction_target_3"];
		} else {
			interaction_targets[2] = 0;
		}

		if (serialized_component.contains("interaction_target_4")) {
			interaction_targets[3] = serialized_component["interaction_target_4"];
		} else {
			interaction_targets[3] = 0;
		}

		if (serialized_component.contains("interaction_target_5")) {
			interaction_targets[4] = serialized_component["interaction_target_5"];
		} else {
			interaction_targets[4] = 0;
		}

		if (serialized_component.contains("single_use")) {
			single_use = serialized_component["single_use"];
		} else {
			single_use = false;
		}

		if (serialized_component.contains("cable_parent")) {
			cable_parent = serialized_component["cable_parent"];
		} else {
			cable_parent = 0;
		}

		if (serialized_component.contains("lever")) {
			lever = serialized_component["lever"];
		} else {
			lever = 0;
		}

		if (serialized_component.contains("is_on")) {
			is_on = serialized_component["is_on"];
		} else {
			is_on = false;
		}

		if (serialized_component.contains("interaction_text")) {
			interaction_text = serialized_component["interaction_text"];
		} else {
			interaction_text = "";
		}

		if (serialized_component.contains("enemy_entity")) {
			enemy_entity = serialized_component["enemy_entity"];
		} else {
			enemy_entity = 0;
		}

		if (serialized_component.contains("enemy_entity2")) {
			enemy_entity2 = serialized_component["enemy_entity2"];
		} else {
			enemy_entity2 = 0;
		}

		if (serialized_component.contains("main_door")) {
			main_door = serialized_component["main_door"];
		} else {
			main_door = 0;
		}

		triggered = false;
	}
};

#endif //SILENCE_INTERACTABLE_COMPONENT_H