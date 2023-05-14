#include "resource_manager.h"
#include "render/common/skinned_model.h"

#define ASSET_PATH "resources/assets_export/"

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string remove_asset_path(std::string_view path) {
	std::size_t asset_path_pos = path.find(ASSET_PATH);

	if (asset_path_pos != std::string::npos) {
		return std::string(path).substr(asset_path_pos + std::strlen(ASSET_PATH));
	}

	return std::string(path);
}

ResourceManager &ResourceManager::get() {
	static ResourceManager instance;
	return instance;
}
void ResourceManager::startup() {
}
void ResourceManager::shutdown() {
}

Handle<Model> ResourceManager::load_model(const char *path) {
	std::string name = path;
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	if (name_to_model.find(name) != name_to_model.end()) {
		return name_to_model[name];
	}

	Model model = {};
	model.load_from_asset(path);

	models.push_back(model);
	Handle<Model> handle = {};
	handle.id = models.size() - 1;

	name_to_model[name] = handle;
	return handle;
}

void ResourceManager::load_texture(const char *path) {
	if (textures.find(path) != textures.end()) {
		return;
	}
	Texture texture = {};
	texture.load_from_asset(asset_path(path));
	textures[path] = texture;
}

Model &ResourceManager::get_model(Handle<Model> handle) {
	return models[handle.id];
}

std::vector<Model> &ResourceManager::get_models() {
	return models;
}

Handle<Model> ResourceManager::get_model_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_model[name];
}

Handle<Animation> ResourceManager::load_animation(const char *path) {
	if (name_to_animation.find(path) != name_to_animation.end()) {
		return name_to_animation[path];
	}

	Animation animation = {};
	animation.load_from_asset(asset_path(path).c_str());

	animations.push_back(animation);
	Handle<Animation> handle = {};
	handle.id = animations.size() - 1;

	name_to_animation[path] = handle;
	return handle;
}

Animation &ResourceManager::get_animation(Handle<Animation> handle) {
	return animations[handle.id];
}

Handle<Animation> ResourceManager::get_animation_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_animation[name];
}

std::vector<Animation> &ResourceManager::get_animations() {
	return animations;
}

SkinnedModel &ResourceManager::get_skinned_model(Handle<SkinnedModel> handle) {
	return skinned_models[handle.id];
}

Handle<SkinnedModel> ResourceManager::get_skinned_model_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_skinned_model[name];
}

std::vector<SkinnedModel> &ResourceManager::get_skinned_models() {
	return skinned_models;
}

Handle<SkinnedModel> ResourceManager::load_skinned_model(const char *path) {
	std::string name = path;
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	if (name_to_skinned_model.find(name) != name_to_skinned_model.end()) {
		return name_to_skinned_model[name];
	}

	SkinnedModel model = {};
	model.load_from_asset(path);

	skinned_models.push_back(model);
	Handle<SkinnedModel> handle = {};
	handle.id = skinned_models.size() - 1;

	name_to_skinned_model[name] = handle;
	return handle;
}
std::string ResourceManager::get_model_name(Handle<Model> handle) {
	for (auto &pair : name_to_model) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
}

std::string ResourceManager::get_skinned_model_name(Handle<SkinnedModel> handle) {
	for (auto &pair : name_to_skinned_model) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
}