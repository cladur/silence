#ifndef SILENCE_STATIC_TAG_COMPONENT_H
#define SILENCE_STATIC_TAG_COMPONENT_H

// This component is intentionally empty because it is only for recognize that entity has static collision
struct StaticTag {
	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		j.push_back(nlohmann::json::object());
		j.back()["static_tag"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("static_tag", j);
	}
};

#endif //SILENCE_STATIC_TAG_COMPONENT_H
