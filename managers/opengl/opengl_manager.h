#ifndef SILENCE_OPENGL_MANAGER_H
#define SILENCE_OPENGL_MANAGER_H

#include "material.h"
#include "mesh.h"
#include "model.h"
#include "render_pass.h"
#include "shader.h"
#include "texture.h"

#include "debug/debug_draw.h"
#include "text/text_draw.h"

#include "camera/camera.h"
#include "opengl/ui/sprite_draw.h"

class OpenglManager {
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

	// Render passes
	UnlitPass unlit_pass;

	static OpenglManager *get();

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

#endif // SILENCE_OPENGL_MANAGER_H