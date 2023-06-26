#ifndef SILENCE_DETECTION_CAMERA_COMPONENT_H
#define SILENCE_DETECTION_CAMERA_COMPONENT_H

#include "fmod_studio.hpp"
struct DetectionCamera {
	float detection_level = 0.0f;
	float friendly_time_left = 0.0f;
	DetectionTarget detection_target = DetectionTarget::NONE;
	bool first_frame = true;
	bool is_active = true;
	bool is_detecting = false;
	glm::quat starting_orientation{};

	bool is_playing = false;
	FMOD::Studio::EventInstance *detection_event = nullptr;

	bool previous_frame_tag_state = false;

	Entity particles_parent = 0;
	Entity camera_light = 0;
	Entity camera_model = 0;

	glm::vec2 default_detection_slider_size = glm::vec2(200.0f, 200.0f);
	float radial_detection_offset = 45.0f;

	DetectionCamera() = default;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["detection_level"] = detection_level;
		serialized_component["is_active"] = is_active;
		serialized_component["particles_parent"] = particles_parent;
		serialized_component["camera_light"] = camera_light;
		serialized_component["camera_model"] = camera_model;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "DetectionCamera";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		if (serialized_component.contains("detection_level")) {
			detection_level = serialized_component["detection_level"];
		} else {
			detection_level = 0.0f;
		}

		if (serialized_component.contains("is_active")) {
			is_active = serialized_component["is_active"];
		} else {
			is_active = true;
		}

		if (serialized_component.contains("particles_parent")) {
			particles_parent = serialized_component["particles_parent"];
		} else {
			particles_parent = 0;
		}

		if (serialized_component.contains("camera_light")) {
			camera_light = serialized_component["camera_light"];
		} else {
			camera_light = 0;
		}

		if (serialized_component.contains("camera_model")) {
			camera_model = serialized_component["camera_model"];
		} else {
			camera_model = 0;
		}
	}
};
#endif //SILENCE_DETECTION_CAMERA_COMPONENT_H