#ifndef SILENCE_NAME_H
#define SILENCE_NAME_H

struct Name {
public:
	std::string name;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["name"] = name;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Name";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		name = serialized_component["name"];
	}

	//constructor
	explicit Name(std::string name) : name(std::move(name)) {
	}

	Name() {
		name = "unnamed";
	}
};

#endif //SILENCE_NAME_H