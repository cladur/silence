#include "animation_instance.h"

#include "render/render_manager.h"
#include "resource/resource_manager.h"

AnimationInstance::AnimationInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_walk.anim");
	resource_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_idle.anim");
	resource_manager.load_animation("agent001/agent_idle_ANIM_GLTF/agent_idle_grab.anim");
	resource_manager.load_animation("agent001/agent_idle_ANIM_GLTF/agent_idle_00_idle.anim");
}

AnimationInstance::AnimationInstance(const char *path) {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation(path);
}

void AnimationInstance::serialize_json(nlohmann::json &serialized_scene) {
	nlohmann::json::object_t serialized_component;
	// RenderManager &render_manager = RenderManager::get();
	// serialized_component["animation_name"] = render_manager.get_skinned_model_name(model_handle);
	// serialized_component["material_type"] = material_type;
	// serialized_scene.push_back(nlohmann::json::object());
	// serialized_scene.back()["component_data"] = serialized_component;
	// serialized_scene.back()["component_name"] = "SkinnedModelInstance";
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "SkinnedModelInstance";
}

void AnimationInstance::deserialize_json(nlohmann::json &serialized_component) {
}
