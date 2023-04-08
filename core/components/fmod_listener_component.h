#ifndef SILENCE_FMOD_LISTENER_COMPONENT_H
#define SILENCE_FMOD_LISTENER_COMPONENT_H

struct FmodListener {
	int listener_id;
	glm::vec3 prev_frame_position{};

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["listener_id"] = listener_id;
		j.push_back(nlohmann::json::object());
		j.back()["fmod_listener"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("fmod_listener", j);
		listener_id = obj["listener_id"];
	}
};

#endif //SILENCE_FMOD_LISTENER_COMPONENT_H
