#ifndef SILENCE_DECAL_COMPONENT_H
#define SILENCE_DECAL_COMPONENT_H

#include "render/common/texture.h"
#include "resource/resource_manager.h"
struct Decal {
	Handle<Texture> albedo;
	Handle<Texture> normal;
	float projection_size = 20.0f;
	float projection_far = 30.0f;
	float projection_near = 0.001f;

	void serialize_json(nlohmann::json &serialized_scene) {
		auto &rm = ResourceManager::get();
		nlohmann::json::object_t serialized_component;
		serialized_component["albedo"] = rm.get_texture_name(albedo);
		serialized_component["normal"] = rm.get_texture_name(normal);
		serialized_component["projection_size"] = projection_size;
		serialized_component["projection_far"] = projection_far;
		serialized_component["projection_near"] = projection_near;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Decal";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		auto &rm = ResourceManager::get();
		std::string albedo_name = serialized_component["albedo"];
		if (!albedo_name.empty()) {
			albedo = rm.load_texture(albedo_name.c_str());
		}
		std::string normal_name = serialized_component["normal"];
		if (!normal_name.empty()) {
			normal = rm.load_texture(normal_name.c_str());
		}
		projection_size = serialized_component["projection_size"];
		projection_far = serialized_component["projection_far"];
		projection_near = serialized_component["projection_near"];
	}
};

#endif //SILENCE_DECAL_COMPONENT_H
