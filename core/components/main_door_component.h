#ifndef SILENCE_MAIN_DOOR_H
#define SILENCE_MAIN_DOOR_H

struct MainDoor {
public:
	Entity left_door = 0;
	Entity right_door = 0;

	int number_of_locks = 0;
	int number_of_locks_opened = 0;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["left_door"] = left_door;
		serialized_component["right_door"] = right_door;
		serialized_component["number_of_locks"] = number_of_locks;
		serialized_component["number_of_locks_opened"] = number_of_locks_opened;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "MainDoor";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		left_door = serialized_component["left_door"];
		right_door = serialized_component["right_door"];
		number_of_locks = serialized_component["number_of_locks"];
		number_of_locks_opened = serialized_component["number_of_locks_opened"];
	}
};

#endif //SILENCE_MAIN_DOOR_H