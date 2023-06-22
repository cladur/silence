#ifndef SILENCE_DECAL_COMPONENT_H
#define SILENCE_DECAL_COMPONENT_H

#include "render/common/texture.h"
#include "resource/resource_manager.h"
struct Decal {
	Handle<Texture> albedo;
	Handle<Texture> normal;
	Handle<Texture> ao_rough_metal;
	bool has_normal = false;
	bool has_ao = false;
	bool has_roughness = false;
	bool has_metalness = false;
	glm::vec4 color = glm::vec4(1.0f);

	void serialize_json(nlohmann::json &serialized_scene) {
		auto &rm = ResourceManager::get();
		nlohmann::json::object_t serialized_component;
		serialized_component["albedo"] = rm.get_texture_name(albedo);
		serialized_component["normal"] = rm.get_texture_name(normal);
		serialized_component["ao_rough_metal"] = rm.get_texture_name(ao_rough_metal);
		serialized_component["color"]["r"] = color.r;
		serialized_component["color"]["g"] = color.g;
		serialized_component["color"]["b"] = color.b;
		serialized_component["color"]["a"] = color.a;
		serialized_component["has_normal"] = has_normal;
		serialized_component["has_ao"] = has_ao;
		serialized_component["has_roughness"] = has_roughness;
		serialized_component["has_metalness"] = has_metalness;
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
		if (serialized_component.contains("ao_rough_metal")) {
			std::string ao_rough_metal_name = serialized_component["ao_rough_metal"];
			if (!normal_name.empty()) {
				ao_rough_metal = rm.load_texture(ao_rough_metal_name.c_str());
			}
		}
		if (serialized_component.contains("color")) {
			color.r = serialized_component["color"]["r"];
			color.g = serialized_component["color"]["g"];
			color.b = serialized_component["color"]["b"];
			color.a = serialized_component["color"]["a"];
		} else {
			color = glm::vec4(1.0f);
		}
		if (serialized_component.contains("has_normal")) {
			has_normal = serialized_component["has_normal"];
		} else {
			has_normal = false;
		}
		if (serialized_component.contains("has_ao")) {
			has_ao = serialized_component["has_ao"];
		} else {
			has_ao = false;
		}
		if (serialized_component.contains("has_roughness")) {
			has_roughness = serialized_component["has_roughness"];
		} else {
			has_roughness = false;
		}
		if (serialized_component.contains("has_metalness")) {
			has_metalness = serialized_component["has_metalness"];
		} else {
			has_metalness = false;
		}
	}
};

#endif //SILENCE_DECAL_COMPONENT_H
