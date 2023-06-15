#ifndef SILENCE_HACKER_DATA_COMPONENT_H
#define SILENCE_HACKER_DATA_COMPONENT_H

#include "audio/event_reference.h"
struct HackerData {
	Entity model;
	Entity camera_pivot;
	Entity scorpion_camera_transform;
	Entity camera;
	bool is_on_camera = false;
	EventReference hack_sound = EventReference("Hacker/hack");

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["model"] = model;
		serialized_component["camera_pivot"] = camera_pivot;
		serialized_component["scorpion_camera_transform"] = scorpion_camera_transform;
		serialized_component["camera"] = camera;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "HackerData";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		model = serialized_component["model"];
		camera_pivot = serialized_component["camera_pivot"];
		camera = serialized_component["camera"];

		if (serialized_component.contains("scorpion_camera_transform")) {
			scorpion_camera_transform = serialized_component["scorpion_camera_transform"];
		} else {
			scorpion_camera_transform = -1;
		}
	}
};

#endif //SILENCE_HACKER_DATA_COMPONENT_H