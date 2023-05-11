#include "animation_instance.h"

#include "render/render_manager.h"

AnimationInstance::AnimationInstance() {
	RenderManager &render_manager = RenderManager::get();
	animation_handle = render_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/00_walk.anim");
	render_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/00_idle.anim");
}

AnimationInstance::AnimationInstance(const char *path) {
	RenderManager &render_manager = RenderManager::get();
	animation_handle = render_manager.load_animation(path);
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
