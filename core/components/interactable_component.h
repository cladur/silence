#ifndef SILENCE_INTERACTABLE_COMPONENT_H
#define SILENCE_INTERACTABLE_COMPONENT_H

enum InteractionType { None, Agent, Hacker };
enum Interaction { NoInteraction, HackerCameraJump, HackerPlatform, Exploding, LightSwitch };

struct Interactable {
	InteractionType type = None;
	Interaction interaction = NoInteraction;

	Entity interaction_target = 0;

	bool triggered = false;
	bool can_interact = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["type"] = type;
		serialized_component["interaction"] = interaction;
		serialized_component["can_interact"] = can_interact;
		serialized_component["interaction_target"] = interaction_target;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Interactable";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		type = serialized_component["type"];
		interaction = serialized_component["interaction"];
		can_interact = serialized_component["can_interact"];
		if (serialized_component.contains("interaction_target")) {
			interaction_target = serialized_component["interaction_target"];
		} else {
			interaction_target = 0;
		}
		triggered = false;
	}
};

#endif //SILENCE_INTERACTABLE_COMPONENT_H