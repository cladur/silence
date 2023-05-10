#ifndef SILENCE_CAMERA_H
#define SILENCE_CAMERA_H

struct Camera {
public:
	float fov = 70.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["fov"] = fov;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Camera";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		fov = serialized_component["fov"];
	}
};

#endif //SILENCE_NAME_H