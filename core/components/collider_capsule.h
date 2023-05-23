#ifndef SILENCE_COLLIDER_CAPSULE_H
#define SILENCE_COLLIDER_CAPSULE_H

struct ColliderCapsule {
	glm::vec3 start;
	glm::vec3 end;
	float radius;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["start"] = nlohmann::json::object();
		serialized_component["start"]["x"] = start.x;
		serialized_component["start"]["y"] = start.y;
		serialized_component["start"]["z"] = start.z;
		serialized_component["end"] = nlohmann::json::object();
		serialized_component["end"]["x"] = end.x;
		serialized_component["end"]["y"] = end.y;
		serialized_component["end"]["z"] = end.z;
		serialized_component["radius"] = radius;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderCapsule";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		start.x = serialized_component["start"]["x"];
		start.y = serialized_component["start"]["y"];
		start.z = serialized_component["start"]["z"];
		end.x = serialized_component["end"]["x"];
		end.y = serialized_component["end"]["y"];
		end.z = serialized_component["end"]["z"];
		radius = serialized_component["radius"];
	}
};
#endif //SILENCE_COLLIDER_CAPSULE_H
