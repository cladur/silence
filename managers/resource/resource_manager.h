#ifndef SILENCE_RESOURCE_MANAGER_H
#define SILENCE_RESOURCE_MANAGER_H

#include "render/common/animation.h"
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
	std::vector<Animation> animations;
	std::unordered_map<std::string, Handle<Animation>> name_to_animation;

	// stuff to remove later
	std::vector<SkinnedModel> skinned_models;
	std::unordered_map<std::string, Handle<SkinnedModel>> name_to_skinned_model;

public:
	static ResourceManager &get();

	void startup();
	void shutdown();

	Handle<Model> load_model(const char *path);
	void load_texture(const char *path);

	Model &get_model(Handle<Model> handle);
	Handle<Model> get_model_handle(std::string name);
	std::string get_model_name(Handle<Model> handle);
	std::vector<Model> &get_models();

	Handle<Animation> load_animation(const char *path);
	Animation &get_animation(Handle<Animation> handle);
	Handle<Animation> get_animation_handle(std::string name);
	std::vector<Animation> &get_animations();

	// stuff to remove later
	Handle<SkinnedModel> load_skinned_model(const char *path);
	SkinnedModel &get_skinned_model(Handle<SkinnedModel> handle);
	Handle<SkinnedModel> get_skinned_model_handle(std::string name);
	std::string get_skinned_model_name(Handle<SkinnedModel> handle);
	std::vector<SkinnedModel> &get_skinned_models();
};

#endif //SILENCE_RESOURCE_MANAGER_H
