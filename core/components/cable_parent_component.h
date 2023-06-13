#ifndef SILENCE_CABLE_PARENT_COMPONENT_H
#define SILENCE_CABLE_PARENT_COMPONENT_H

enum class CableState {
	OFF,
	ON
};

struct CableParent {
	glm::vec3 on_color, off_color;
	CableState state = CableState::OFF;
	bool highlighted_on_off = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["on_color"] = nlohmann::json::object();
		serialized_component["on_color"]["r"] = on_color.r;
		serialized_component["on_color"]["g"] = on_color.g;
		serialized_component["on_color"]["b"] = on_color.b;

		serialized_component["off_color"] = nlohmann::json::object();
		serialized_component["off_color"]["r"] = off_color.r;
		serialized_component["off_color"]["g"] = off_color.g;
		serialized_component["off_color"]["b"] = off_color.b;

		serialized_component["state"] = static_cast<int>(state);

		serialized_component["highlighted_on_off"] = highlighted_on_off;

		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "CableParent";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		on_color.x = serialized_component["on_color"]["r"];
		on_color.y = serialized_component["on_color"]["g"];
		on_color.z = serialized_component["on_color"]["b"];

		off_color.x = serialized_component["off_color"]["r"];
		off_color.y = serialized_component["off_color"]["g"];
		off_color.z = serialized_component["off_color"]["b"];

		state = static_cast<CableState>(serialized_component["state"]);

		highlighted_on_off = serialized_component["highlighted_on_off"];

		state = static_cast<CableState>(serialized_component["state"]);
	}
};

#endif //SILENCE_CABLE_PARENT_COMPONENT_H
