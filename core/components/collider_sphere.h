#ifndef SILENCE_COLLIDER_SPHERE_H
#define SILENCE_COLLIDER_SPHERE_H

struct ColliderSphere {
	// Center of collider
	glm::vec3 center;
	float radius;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["center"] = nlohmann::json::object();
		serialized_component["center"]["x"] = center.x;
		serialized_component["center"]["y"] = center.y;
		serialized_component["center"]["z"] = center.z;
		serialized_component["radius"] = radius;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderSphere";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		center.x = serialized_component["center"]["x"];
		center.y = serialized_component["center"]["y"];
		center.z = serialized_component["center"]["z"];
		radius = serialized_component["radius"];
	}
};

#endif //SILENCE_COLLIDER_SPHERE_H
