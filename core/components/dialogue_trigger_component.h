#ifndef SILENCE_DIALOGUE_TRIGGER_COMPONENT_H
#define SILENCE_DIALOGUE_TRIGGER_COMPONENT_H

struct DialogueTrigger {
	std::string dialogue_id;
	bool triggered = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["dialogue_id"] = dialogue_id;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "DialogueTrigger";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		if (serialized_component.contains("dialogue_id")) {
			dialogue_id = serialized_component["dialogue_id"];
		}
	}
};

#endif //SILENCE_DIALOGUE_TRIGGER_COMPONENT_H
