#ifndef SILENCE_COLLIDER_TAG_COMPONENT_H
#define SILENCE_COLLIDER_TAG_COMPONENT_H

// This component is intentionally empty because it is only for recognize that entity has collision component
struct ColliderTag {
	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		j.push_back(nlohmann::json::object());
		j.back()["collider_tag"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("collider_tag", j);
	}
};

#endif //SILENCE_COLLIDER_TAG_COMPONENT_H
