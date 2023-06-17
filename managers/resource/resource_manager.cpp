#include "resource_manager.h"
#include "render/common/skinned_model.h"
#include <spdlog/spdlog.h>

#define ASSET_PATH "resources/assets_export/"
#define ASSET_PATH_2 "resources\\assets_export\\"

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string remove_asset_path(std::string_view path) {
	std::size_t asset_path_pos = path.find(ASSET_PATH);

	if (asset_path_pos != std::string::npos) {
		return std::string(path).substr(asset_path_pos + std::strlen(ASSET_PATH));
	}

	asset_path_pos = path.find(ASSET_PATH_2);

	if (asset_path_pos != std::string::npos) {
		return std::string(path).substr(asset_path_pos + std::strlen(ASSET_PATH_2));
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

Handle<Texture> ResourceManager::load_texture(const char *path, bool repeat) {
	std::string name = path;
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	found_asset_path |= name.find(ASSET_PATH_2) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	if (name_to_texture.find(name) != name_to_texture.end()) {
		return name_to_texture[name];
	}

	Texture texture = {};
	texture.load_from_asset(asset_path(name), false, repeat);

	textures.push_back(texture);
	Handle<Texture> handle = {};
	handle.id = textures.size() - 1;

	name_to_texture[name] = handle;
	return handle;
}

Texture &ResourceManager::get_texture(Handle<Texture> handle) {
	return textures[handle.id];
}

Handle<Texture> ResourceManager::get_texture_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	found_asset_path |= name.find(ASSET_PATH_2) != std::string::npos;

	if (found_asset_path) {
		name = remove_asset_path(name);
	}

	return name_to_texture[name];
}

std::string ResourceManager::get_texture_name(Handle<Texture> handle) {
	for (auto &pair : name_to_texture) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
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
	std::string name = path;
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	if (name_to_animation.find(name) != name_to_animation.end()) {
		return name_to_animation[name];
	}

	Animation animation = {};
	animation.load_from_asset(path);

	animations.push_back(animation);
	Handle<Animation> handle = {};
	handle.id = animations.size() - 1;

	name_to_animation[name] = handle;
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
	if (!name_to_animation.contains(name)) {
		SPDLOG_WARN("not found animation with name: {}", name);
	}
	return name_to_animation[name];
}

std::string ResourceManager::get_animation_name(Handle<Animation> handle) {
	for (auto &pair : name_to_animation) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
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