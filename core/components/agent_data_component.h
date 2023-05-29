#ifndef SILENCE_AGENT_DATA_COMPONENT_H
#define SILENCE_AGENT_DATA_COMPONENT_H

struct AgentData {
	Entity model;
	Entity camera_pivot;
	Entity spring_arm;
	Entity camera;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["model"] = model;
		serialized_component["camera_pivot"] = camera_pivot;
		serialized_component["spring_arm"] = spring_arm;
		serialized_component["camera"] = camera;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "AgentData";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		model = serialized_component["model"];
		camera_pivot = serialized_component["camera_pivot"];
		if (serialized_component.contains("spring_arm")) {
			spring_arm = serialized_component["spring_arm"];
		} else {
			spring_arm = 0;
		}
		camera = serialized_component["camera"];
	}
};

#endif //SILENCE_AGENT_DATA_COMPONENT_H
