#include "animation_instance.h"

#include "render/render_manager.h"

AnimationInstance::AnimationInstance() {
	RenderManager &render_manager = RenderManager::get();
	animation_handle = render_manager.load_animation("scorpion/scorpion_ANIM_GLTF/00_walk.anim");
}

AnimationInstance::AnimationInstance(const char *path) {
	RenderManager &render_manager = RenderManager::get();
	animation_handle = render_manager.load_animation(path);
}

void AnimationInstance::serialize_json(nlohmann::json &j) {
	// TODO good serialization
	nlohmann::json::object_t obj;
	obj["animation_handle"] = animation_handle.id;
	j.push_back(nlohmann::json::object());
	j.back()["animation_instance"] = obj;
}

void AnimationInstance::deserialize_json(nlohmann::json &j) {
	nlohmann::json obj = Serializer::get_data("animation_instance", j);
	animation_handle = static_cast<Handle<Animation>>(obj["animation_handle"]);
}
