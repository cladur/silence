#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include "managers/render/common/material.h"
#include "managers/render/common/mesh.h"
#include "managers/render/common/model.h"
#include "managers/render/common/render_pass.h"
#include "managers/render/common/shader.h"
#include "managers/render/common/texture.h"

#include "debug/debug_draw.h"
#include "transparent_elements/text/text_draw.h"

#include "camera/camera.h"
#include "render/transparent_elements/transparent_draw.h"
#include "transparent_elements/ui/sprite_draw.h"

class RenderManager {
private:
	std::unordered_map<std::string, Texture> textures;
	std::vector<Model> models;
	std::unordered_map<std::string, Handle<Model>> name_to_model;

	std::vector<ModelInstance> model_instances;

public:
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 camera_pos;

	TextDraw text_draw;
	DebugDraw debug_draw;
	SpriteDraw sprite_draw;
	TransparentDraw transparent_draw;

	// Render passes
	UnlitPass unlit_pass;
	PBRPass pbr_pass;
	SkyboxPass skybox_pass;

	static RenderManager *get();

	void startup();
	void shutdown();
	void draw();

	void load_model(const char *path);
	void load_texture(const char *path);

	Model &get_model(Handle<Model> handle);
	ModelInstance &get_model_instance(Handle<ModelInstance> handle);

	Handle<ModelInstance> add_instance(
			const char *path, MaterialType material_type = MATERIAL_TYPE_PBR, bool in_shadow_pass = true);
	void remove_instance(Handle<ModelInstance> handle);

	void update_instance_passes(Handle<ModelInstance> handle, MaterialType material_type, bool in_shadow_pass);
	void update_instance_transform(Handle<ModelInstance> handle, glm::mat4 const &transform);
};

#endif // SILENCE_RENDER_MANAGER_H