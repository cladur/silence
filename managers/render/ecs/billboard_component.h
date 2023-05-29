#ifndef SILENCE_BILLBOARD_COMPONENT_H
#define SILENCE_BILLBOARD_COMPONENT_H

struct Billboard {
	Handle<Texture> texture;
	glm::vec3 center_offset;
	glm::vec3 position_offset;
	glm::vec2 scale;
	glm::vec4 color;
	bool use_camera_right = false;
	bool first_frame = true;
	std::string ui_name;

	Billboard() = default;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["texture"] = ResourceManager::get().get_texture_name(texture);
		serialized_component["center_offset"] = nlohmann::json::object();
		serialized_component["center_offset"]["x"] = center_offset.x;
		serialized_component["center_offset"]["y"] = center_offset.y;
		serialized_component["center_offset"]["z"] = center_offset.z;
		serialized_component["position_offset"] = nlohmann::json::object();
		serialized_component["position_offset"]["x"] = position_offset.x;
		serialized_component["position_offset"]["y"] = position_offset.y;
		serialized_component["position_offset"]["z"] = position_offset.z;
		serialized_component["scale"] = nlohmann::json::object();
		serialized_component["scale"]["x"] = scale.x;
		serialized_component["scale"]["y"] = scale.y;
		serialized_component["color"] = nlohmann::json::object();
		serialized_component["color"]["r"] = color.r;
		serialized_component["color"]["g"] = color.g;
		serialized_component["color"]["b"] = color.b;
		serialized_component["color"]["a"] = color.a;
		serialized_component["use_camera_right"] = use_camera_right;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Billboard";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		std::string texture_name = serialized_component["texture"];
		texture = ResourceManager::get().load_texture(asset_path(texture_name).c_str());
		center_offset.x = serialized_component["center_offset"]["x"];
		center_offset.y = serialized_component["center_offset"]["y"];
		center_offset.z = serialized_component["center_offset"]["z"];
		position_offset.x = serialized_component["position_offset"]["x"];
		position_offset.y = serialized_component["position_offset"]["y"];
		position_offset.z = serialized_component["position_offset"]["z"];
		scale.x = serialized_component["scale"]["x"];
		scale.y = serialized_component["scale"]["y"];
		color.r = serialized_component["color"]["r"];
		color.g = serialized_component["color"]["g"];
		color.b = serialized_component["color"]["b"];
		color.a = serialized_component["color"]["a"];
		use_camera_right = serialized_component["use_camera_right"];
	}
};

#endif //SILENCE_BILLBOARD_COMPONENT_H
