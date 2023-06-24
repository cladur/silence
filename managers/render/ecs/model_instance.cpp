#include "model_instance.h"

#include "render/render_manager.h"
#include "resource/resource_manager.h"
#include "types.h"

ModelInstance::ModelInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	model_handle = resource_manager.load_model(asset_path("items/wooden_box/wooden_box.mdl").c_str());
	material_type = MaterialType::Default;
}

ModelInstance::ModelInstance(const char *path, MaterialType material_type, bool scale_uv_with_transform) {
	ResourceManager &resource_manager = ResourceManager::get();
	model_handle = resource_manager.load_model(path);
	this->material_type = material_type;
	this->scale_uv_with_transform = scale_uv_with_transform;
}
void ModelInstance::serialize_json(nlohmann::json &serialized_scene) {
	nlohmann::json::object_t serialized_component;
	ResourceManager &resource_manager = ResourceManager::get();
	serialized_component["uv_scale"] = nlohmann::json::object();
	serialized_component["uv_scale"]["x"] = uv_scale.x;
	serialized_component["uv_scale"]["y"] = uv_scale.y;
	serialized_component["model_name"] = resource_manager.get_model_name(model_handle);
	serialized_component["material_type"] = material_type;
	serialized_component["scale_uv_with_transform"] = scale_uv_with_transform;
	serialized_component["in_shadow_pass"] = in_shadow_pass;
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "ModelInstance";
}

void ModelInstance::deserialize_json(nlohmann::json &serialized_component) {
	std::string model_name = serialized_component["model_name"];
	ResourceManager &resource_manager = ResourceManager::get();

	model_handle = resource_manager.load_model(asset_path(model_name).c_str());
	material_type = static_cast<MaterialType>(serialized_component["material_type"]);
	if (serialized_component.contains("scale_uv_with_transform")) {
		scale_uv_with_transform = serialized_component["scale_uv_with_transform"];
	} else {
		scale_uv_with_transform = false;
	}

	if (serialized_component.contains("in_shadow_pass")) {
		in_shadow_pass = serialized_component["in_shadow_pass"];
	} else {
		in_shadow_pass = true;
	}

	// if (serialized_component.contains("uv_scale")) {
	// 	uv_scale.x = serialized_component["uv_scale"]["x"];
	// 	uv_scale.y = serialized_component["uv_scale"]["y"];
	// } else {
	// 	uv_scale = glm::vec2(1.0f, 1.0f);
	// }
}