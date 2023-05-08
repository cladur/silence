#include "resource_manager.h"

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
