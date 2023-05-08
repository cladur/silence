#ifndef SILENCE_RESOURCE_MANAGER_H
#define SILENCE_RESOURCE_MANAGER_H

#include "render/common/material.h"
#include "render/common/mesh.h"
#include "render/common/model.h"
#include "render/common/shader.h"
#include "render/common/texture.h"

template <typename T> struct Handle {
	uint32_t id;
};

class ResourceManager {
private:
	std::unordered_map<std::string, Texture> textures;
	std::vector<Model> models;
	std::unordered_map<std::string, Handle<Model>> name_to_model;

public:
	static ResourceManager &get();

	void startup();
	void shutdown();

	Handle<Model> load_model(const char *path);
	void load_texture(const char *path);

	Model &get_model(Handle<Model> handle);
	Handle<Model> get_model_handle(std::string name);
	std::vector<Model> &get_models();
};

#endif //SILENCE_RESOURCE_MANAGER_H
