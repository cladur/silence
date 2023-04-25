#ifndef SILENCE_GRAVITY_H
#define SILENCE_GRAVITY_H

struct Gravity {
	glm::vec3 force;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["force"] = nlohmann::json::object();
		obj["force"]["x"] = force.x;
		obj["force"]["y"] = force.y;
		obj["force"]["z"] = force.z;
		j.push_back(nlohmann::json::object());
		j.back()["gravity"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("gravity", j);
		force.x = obj["force"]["x"];
		force.y = obj["force"]["y"];
		force.z = obj["force"]["z"];
	}
};

#endif //SILENCE_GRAVITY_H
