#ifndef SILENCE_RIGIDBODY_H
#define SILENCE_RIGIDBODY_H

struct RigidBody {
	glm::vec3 velocity;
	glm::vec3 acceleration;
	float mass = 1.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["velocity"] = nlohmann::json::object();
		serialized_component["acceleration"] = nlohmann::json::object();
		serialized_component["velocity"]["x"] = velocity.x;
		serialized_component["velocity"]["y"] = velocity.y;
		serialized_component["velocity"]["z"] = velocity.z;
		serialized_component["acceleration"]["x"] = acceleration.x;
		serialized_component["acceleration"]["y"] = acceleration.y;
		serialized_component["acceleration"]["z"] = acceleration.z;
		serialized_component["mass"] = mass;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "RigidBody";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		velocity.x = serialized_component["velocity"]["x"];
		velocity.y = serialized_component["velocity"]["y"];
		velocity.z = serialized_component["velocity"]["z"];
		acceleration.x = serialized_component["acceleration"]["x"];
		acceleration.y = serialized_component["acceleration"]["y"];
		acceleration.z = serialized_component["acceleration"]["z"];
		mass = serialized_component["mass"];
	}
};

#endif //SILENCE_RIGIDBODY_H