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
	serialized_component["model_handle"] = model_handle.id;
	serialized_component["material_type"] = material_type;
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "ModelInstance";
}

void ModelInstance::deserialize_json(nlohmann::json &serialized_component) {
	model_handle = static_cast<Handle<Model>>(serialized_component["model_handle"]);
	material_type = static_cast<MaterialType>(serialized_component["material_type"]);
}
