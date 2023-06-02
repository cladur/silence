#ifndef SILENCE_LIGHT_H
#define SILENCE_LIGHT_H

enum class LightType : uint8_t { POINT_LIGHT = 0, DIRECTIONAL_LIGHT = 1, SPOT_LIGHT = 2 };

struct Light {
public:
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
	LightType type = LightType::POINT_LIGHT;
	float intensity = 1.0f;
	float cutoff = 12.5f;
	float outer_cutoff = 5.0f;
	bool cast_shadow = false;
	// Do not serialize this
	std::vector<glm::mat4> light_space;
	uint32_t shadow_map_id = 0;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["color"] = nlohmann::json::object();
		serialized_component["color"]["r"] = color.r;
		serialized_component["color"]["g"] = color.g;
		serialized_component["color"]["b"] = color.b;
		serialized_component["intensity"] = intensity;
		serialized_component["cutoff"] = cutoff;
		serialized_component["outer_cutoff"] = outer_cutoff;
		serialized_component["type"] = type;
		serialized_component["cast_shadow"] = cast_shadow;
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

		if (serialized_component.contains("cutoff")) {
			cutoff = serialized_component["cutoff"];
		} else {
			cutoff = 12.5f;
		}

		if (serialized_component.contains("outer_cutoff")) {
			outer_cutoff = serialized_component["outer_cutoff"];
		} else {
			outer_cutoff = 5.0f;
		}

		if (serialized_component.contains("cast_shadow")) {
			cast_shadow = serialized_component["cast_shadow"];
		} else {
			cast_shadow = false;
		}
	}
};

#endif //SILENCE_LIGHT_H