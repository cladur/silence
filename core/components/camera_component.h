#ifndef SILENCE_CAMERA_H
#define SILENCE_CAMERA_H

struct Camera {
public:
	float fov = 70.0f;
	bool right_side = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["fov"] = fov;
		serialized_component["right_side"] = right_side;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Camera";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		fov = serialized_component["fov"];
		if (serialized_component.contains("right_side")) {
			right_side = serialized_component["right_side"];
		} else {
			right_side = false;
		}
	}
};

#endif //SILENCE_NAME_H