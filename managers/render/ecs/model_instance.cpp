#include "model_instance.h"

#include "render/render_manager.h"

ModelInstance::ModelInstance() {
	RenderManager &render_manager = RenderManager::get();
	model_handle = render_manager.load_model("woodenBox/woodenBox.mdl");
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
	// obj["mesh"] = "box";
	// obj["material"] = "default_mesh";
	j.push_back(nlohmann::json::object());
	j.back()["model_instance"] = obj;
}

void ModelInstance::deserialize_json(nlohmann::json &j) {
	nlohmann::json obj = Serializer::get_data("render_handle", j);
	// mesh = render_manager.get_mesh(obj["mesh"]);
	// material = render_manager.get_material(obj["material"]);
}
