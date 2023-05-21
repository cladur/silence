#ifndef SILENCE_LIGHT_H
#define SILENCE_LIGHT_H

enum LightType { POINT_LIGHT, DIRECTIONAL_LIGHT, SPOT_LIGHT };

struct Light {
public:
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	LightType type = POINT_LIGHT;
	float intensity = 1.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["color"] = nlohmann::json::object();
		serialized_component["color"]["r"] = color.r;
		serialized_component["color"]["g"] = color.g;
		serialized_component["color"]["b"] = color.b;
		serialized_component["intensity"] = intensity;
		serialized_component["type"] = type;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Light";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		color.r = serialized_component["color"]["r"];
		color.g = serialized_component["color"]["g"];
		color.b = serialized_component["color"]["b"];
		type = serialized_component["type"];
		if (serialized_component.contains("intensity")) {
			intensity = serialized_component["intensity"];
		} else {
			intensity = 1.0f;
		}
	}
};

#endif //SILENCE_LIGHT_H