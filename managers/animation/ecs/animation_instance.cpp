#include "animation_instance.h"

#include "render/render_manager.h"
#include "resource/resource_manager.h"

AnimationInstance::AnimationInstance() {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle =
			resource_manager.load_animation(asset_path("scorpion/scorpion_ANIM_GLTF/scorpion_walk.anim").c_str());
	resource_manager.load_animation(asset_path("scorpion/scorpion_ANIM_GLTF/scorpion_idle.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_crouch.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_crouch_idle.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_walk_stealthy.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_idle.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_idle.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_interaction.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_jump_up.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_grab_and_stab.anim").c_str());
	resource_manager.load_animation(asset_path("agent/agent_ANIM_GLTF/agent_stab.anim").c_str());
	resource_manager.load_animation(asset_path("enemy/enemy_ANIM_GLTF/enemy_idle.anim").c_str());
	resource_manager.load_animation(asset_path("enemy/enemy_ANIM_GLTF/enemy_death.anim").c_str());
	resource_manager.load_animation(asset_path("enemy/enemy_ANIM_GLTF/enemy_down_to_aim.anim").c_str());
	resource_manager.load_animation(asset_path("enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim").c_str());
}

AnimationInstance::AnimationInstance(const char *path) {
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation(asset_path(path).c_str());
}

void AnimationInstance::serialize_json(nlohmann::json &serialized_scene) {
	nlohmann::json::object_t serialized_component;
	ResourceManager &resource_manager = ResourceManager::get();
	serialized_component["animation_name"] = resource_manager.get_animation_name(animation_handle);
	serialized_component["ticks_per_second"] = ticks_per_second;
	serialized_scene.push_back(nlohmann::json::object());
	serialized_scene.back()["component_data"] = serialized_component;
	serialized_scene.back()["component_name"] = "AnimationInstance";
}

void AnimationInstance::deserialize_json(nlohmann::json &serialized_component) {
	std::string animation_name = serialized_component["animation_name"];
	ticks_per_second = serialized_component["ticks_per_second"];
	ResourceManager &resource_manager = ResourceManager::get();
	animation_handle = resource_manager.load_animation(asset_path(animation_name).c_str());
}
