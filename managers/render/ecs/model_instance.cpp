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
void ModelInstance::serialize_json(nlohmann::json &j) {
	// TODO good serialization
	nlohmann::json::object_t obj;
	obj["model_handle"] = model_handle.id;
	obj["material_type"] = material_type;
	j.push_back(nlohmann::json::object());
	j.back()["model_instance"] = obj;
}

void ModelInstance::deserialize_json(nlohmann::json &j) {
	nlohmann::json obj = Serializer::get_data("model_instance", j);
	model_handle = static_cast<Handle<Model>>(obj["model_handle"]);
	material_type = static_cast<MaterialType>(obj["material_type"]);
}
