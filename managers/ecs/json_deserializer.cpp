#include "json_deserializer.h"

void JsonDeserializer::deserialize_entity_json(nlohmann::json &j, Entity entity) {
	std::queue<int> components_on_entity{};
	get_components_on_entity(j, components_on_entity);
}

void JsonDeserializer::deserialize_scene_json(nlohmann::json &j) {
}

void JsonDeserializer::get_components_on_entity(nlohmann::json &j,
		std::queue<int> &components_on_entity) { // NOLINT(readability-convert-member-functions-to-static)
	std::string signature = j["signature"];
	for (int i = 0; i < signature.length(); i++) {
		if (signature[i] == '1') {
			components_on_entity.push(i);
		}
	}
}
