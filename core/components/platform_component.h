#ifndef SILENCE_PLATFORM_COMPONENT_H
#define SILENCE_PLATFORM_COMPONENT_H

#include "fmod_studio.hpp"
#include <glm/fwd.hpp>

struct Platform {
	glm::vec3 starting_position;
	glm::vec3 ending_position;
	glm::vec3 change_vector;

	float speed = 1.0f;

	FMOD::Studio::EventInstance *event_instance = nullptr;
	bool first_frame = true;
	bool is_playing = false;

	bool is_moving = false;
	bool is_door = false;
	//bool at_first_position = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["first_position"] = nlohmann::json::object();
		serialized_component["second_position"] = nlohmann::json::object();
		serialized_component["first_position"]["x"] = starting_position.x;
		serialized_component["first_position"]["y"] = starting_position.y;
		serialized_component["first_position"]["z"] = starting_position.z;
		serialized_component["second_position"]["x"] = ending_position.x;
		serialized_component["second_position"]["y"] = ending_position.y;
		serialized_component["second_position"]["z"] = ending_position.z;
		serialized_component["is_door"] = is_door;
		serialized_component["speed"] = speed;

		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Platform";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		is_moving = false;
		starting_position.x = serialized_component["first_position"]["x"];
		starting_position.y = serialized_component["first_position"]["y"];
		starting_position.z = serialized_component["first_position"]["z"];
		ending_position.x = serialized_component["second_position"]["x"];
		ending_position.y = serialized_component["second_position"]["y"];
		ending_position.z = serialized_component["second_position"]["z"];
		speed = serialized_component["speed"];
		if (serialized_component.contains("is_door")) {
			is_door = serialized_component["is_door"];
		} else {
			is_door = false;
		}
		//at_first_position = serialized_component["at_first_position"];
	}
};

#endif //SILENCE_PLATFORM_COMPONENT_H
