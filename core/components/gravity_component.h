#ifndef SILENCE_GRAVITY_H
#define SILENCE_GRAVITY_H

struct Gravity {
	glm::vec3 force;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["force"] = nlohmann::json::object();
		serialized_component["force"]["x"] = force.x;
		serialized_component["force"]["y"] = force.y;
		serialized_component["force"]["z"] = force.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Gravity";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		force.x = serialized_component["force"]["x"];
		force.y = serialized_component["force"]["y"];
		force.z = serialized_component["force"]["z"];
	}
};

#endif //SILENCE_GRAVITY_H
