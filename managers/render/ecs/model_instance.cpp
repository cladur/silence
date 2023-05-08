#include "model_instance.h"

#include "render/render_manager.h"

ModelInstance::ModelInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	model_handle = resource_manager.load_model("woodenBox/woodenBox.mdl");
	material_type = MaterialType::Default;
}

ModelInstance::ModelInstance(const char *path, MaterialType material_type) {
	ResourceManager &resource_manager = ResourceManager::get();
	model_handle = resource_manager.load_model(path);
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
	// mesh = render_manager.get_mesh(obj["mesh"]);
	// material = render_manager.get_material(obj["material"]);
	//	nlohmann::json obj = Serializer::get_data("transform", j);
	//
	//	position.x = obj["position"]["x"];
	//	position.y = obj["position"]["y"];
	//	position.z = obj["position"]["z"];
	//	orientation.x = obj["orientation"]["x"];
	//	orientation.y = obj["orientation"]["y"];
	//	orientation.z = obj["orientation"]["z"];
	//	orientation.w = obj["orientation"]["w"];
	//	scale.x = obj["scale"]["x"];
	//	scale.y = obj["scale"]["y"];
	//	scale.z = obj["scale"]["z"];
}
