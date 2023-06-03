#include "skinned_model_instance.h"

#include "animation/animation_manager.h"
#include "render/render_manager.h"
#include "resource/resource_manager.h"

SkinnedModelInstance::SkinnedModelInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	model_handle = resource_manager.load_skinned_model(asset_path("scorpion/scorpion.skinmdl").c_str());
	material_type = MaterialType::Default;

	glGenBuffers(1, &skinning_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, skinning_buffer);
	std::vector<glm::mat4> m(animation_manager.MAX_BONE_COUNT, glm::mat4(1.0f));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * animation_manager.MAX_BONE_COUNT, m.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

SkinnedModelInstance::SkinnedModelInstance(const char *path, MaterialType material_type) {
	ResourceManager &resource_manager = ResourceManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	model_handle = resource_manager.load_skinned_model(asset_path(path).c_str());
	this->material_type = material_type;

	glGenBuffers(1, &skinning_buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, skinning_buffer);
	std::vector<glm::mat4> m(animation_manager.MAX_BONE_COUNT, glm::mat4(1.0f));
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * animation_manager.MAX_BONE_COUNT, m.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SkinnedModelInstance::serialize_json(nlohmann::json &serialized_scene) {
	nlohmann::json::object_t serialized_component;
	ResourceManager &resource_manager = ResourceManager::get();
	serialized_component["model_name"] = resource_manager.get_skinned_model_name(model_handle);
	serialized_component["material_type"] = material_type;
	serialized_component["in_shadow_pass"] = in_shadow_pass;
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "SkinnedModelInstance";
}

void SkinnedModelInstance::deserialize_json(nlohmann::json &serialized_component) {
	std::string model_name = serialized_component["model_name"];
	ResourceManager &resource_manager = ResourceManager::get();
	model_handle = resource_manager.load_skinned_model(asset_path(model_name).c_str());
	material_type = static_cast<MaterialType>(serialized_component["material_type"]);
	if (serialized_component.contains("in_shadow_pass")) {
		in_shadow_pass = serialized_component["in_shadow_pass"];
	} else {
		in_shadow_pass = true;
	}
	in_shadow_pass = true;
}

void SkinnedModelInstance::release() {
	// added this method because destructor is called multiply times when adding to entity
	glDeleteBuffers(1, &skinning_buffer);
}
