#ifndef SILENCE_RIGIDBODY_H
#define SILENCE_RIGIDBODY_H

struct RigidBody {
	glm::vec3 velocity;
	glm::vec3 acceleration;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["velocity"] = nlohmann::json::object();
		obj["acceleration"] = nlohmann::json::object();
		obj["velocity"]["x"] = velocity.x;
		obj["velocity"]["y"] = velocity.y;
		obj["velocity"]["z"] = velocity.z;
		obj["acceleration"]["x"] = acceleration.x;
		obj["acceleration"]["y"] = acceleration.y;
		obj["acceleration"]["z"] = acceleration.z;
		j.push_back(nlohmann::json::object());
		j.back()["rigidbody"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
	}
};

#endif //SILENCE_RIGIDBODY_H