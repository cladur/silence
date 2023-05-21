#ifndef SILENCE_PLATFORM_COMPONENT_H
#define SILENCE_PLATFORM_COMPONENT_H

struct Platform {
	glm::vec3 first_position;
	glm::vec3 second_position;

	float speed = 1.0f;

	bool is_moving = false;
	bool at_first_position = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["position"] = nlohmann::json::object();
		serialized_component["orientation"] = nlohmann::json::object();
		serialized_component["scale"] = nlohmann::json::object();
		serialized_component["first_position"]["x"] = first_position.x;
		serialized_component["first_position"]["y"] = first_position.y;
		serialized_component["first_position"]["z"] = first_position.z;
		serialized_component["second_position"]["x"] = second_position.x;
		serialized_component["second_position"]["y"] = second_position.y;
		serialized_component["second_position"]["z"] = second_position.z;
		serialized_component["speed"] = speed;
		serialized_component["at_first_position"] = at_first_position;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Platform";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		is_moving = false;
		first_position.x = serialized_component["first_position"]["x"];
		first_position.y = serialized_component["first_position"]["y"];
		first_position.z = serialized_component["first_position"]["z"];
		second_position.x = serialized_component["second_position"]["x"];
		second_position.y = serialized_component["second_position"]["y"];
		second_position.z = serialized_component["second_position"]["z"];
		speed = serialized_component["speed"];
		at_first_position = serialized_component["at_first_position"];
	}
};

#endif //SILENCE_PLATFORM_COMPONENT_H
