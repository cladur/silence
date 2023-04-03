#ifndef SILENCE_GRAVITY_H
#define SILENCE_GRAVITY_H

struct Gravity {
	glm::vec3 force;

	void serialize(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["force"] = nlohmann::json::object();
		obj["force"]["x"] = force.x;
		obj["force"]["y"] = force.y;
		obj["force"]["z"] = force.z;
		j.push_back(nlohmann::json::object());
		j.back()["gravity"] = obj;
	}
};

#endif //SILENCE_GRAVITY_H
