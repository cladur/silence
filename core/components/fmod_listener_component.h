#ifndef SILENCE_FMOD_LISTENER_COMPONENT_H
#define SILENCE_FMOD_LISTENER_COMPONENT_H

struct FmodListener {
	int listener_id;
	glm::vec3 prev_frame_position{};

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["listener_id"] = listener_id;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "FmodListener";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		listener_id = serialized_component["listener_id"];
	}
};

#endif //SILENCE_FMOD_LISTENER_COMPONENT_H
