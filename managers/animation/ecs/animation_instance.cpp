#include "animation_instance.h"

#include "render/render_manager.h"
#include "resource/resource_manager.h"

AnimationInstance::AnimationInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_walk.anim");
	resource_manager.load_animation("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_idle.anim");
	resource_manager.load_animation("agent/agent_ANIM_GLTF/agent_crouch.anim");
	resource_manager.load_animation("agent/agent_ANIM_GLTF/agent_walk.anim");
	resource_manager.load_animation("agent/agent_ANIM_GLTF/agent_idle.anim");
	resource_manager.load_animation("agent/agent_ANIM_GLTF/agent_interaction.anim");
	
}

AnimationInstance::AnimationInstance(const char *path) {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation(path);
}

void AnimationInstance::serialize_json(nlohmann::json &serialized_scene) {
	nlohmann::json::object_t serialized_component;
	ResourceManager &resource_manager = ResourceManager::get();
	serialized_component["animation_name"] = resource_manager.get_animation_name(animation_handle);
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "AnimationInstance";
}

void AnimationInstance::deserialize_json(nlohmann::json &serialized_component) {
	std::string animation_name = serialized_component["animation_name"];
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation(animation_name.c_str());
}
