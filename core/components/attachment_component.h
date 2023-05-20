#ifndef SILENCE_ATTACHMENT_COMPONENT_H
#define SILENCE_ATTACHMENT_COMPONENT_H

struct Attachment {
	std::string bone_name = "";
	Entity holder = -1;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["bone_name"] = bone_name;
		serialized_component["holder"] = holder;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Attachment";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		bone_name = serialized_component["bone_name"];
		holder = serialized_component["holder"];
	}
};
#endif //SILENCE_ATTACHMENT_COMPONENT_H
