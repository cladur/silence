#include "model_instance.h"

#include "render/render_manager.h"
#include "types.h"

ModelInstance::ModelInstance() {
	RenderManager &render_manager = RenderManager::get();
	model_handle = render_manager.load_model(asset_path("woodenBox/woodenBox.mdl").c_str());
	material_type = MaterialType::Default;
}

ModelInstance::ModelInstance(const char *path, MaterialType material_type) {
	RenderManager &render_manager = RenderManager::get();
	model_handle = render_manager.load_model(path);
	this->material_type = material_type;
}
void ModelInstance::serialize_json(nlohmann::json &serialized_scene) {
	// TODO good serialization
	nlohmann::json::object_t serialized_component;
	RenderManager &render_manager = RenderManager::get();
	serialized_component["model_name"] = render_manager.get_model_name(model_handle);
	serialized_component["material_type"] = material_type;
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "ModelInstance";
}

void ModelInstance::deserialize_json(nlohmann::json &serialized_component) {
	std::string model_name = serialized_component["model_name"];
	RenderManager &render_manager = RenderManager::get();
	model_handle = render_manager.load_model(asset_path(model_name).c_str());
	material_type = static_cast<MaterialType>(serialized_component["material_type"]);
}