#ifndef SILENCE_RENDER_MANAGER_H
#define SILENCE_RENDER_MANAGER_H

#include "common/framebuffer.h"
#include "common/material.h"
#include "common/mesh.h"
#include "common/model.h"
#include "common/render_pass.h"
#include "common/shader.h"
#include "common/texture.h"

#include "components/transform_component.h"
#include "debug/debug_draw.h"
#include "text/text_draw.h"

#include "camera/camera.h"

class RenderManager {
private:
	std::unordered_map<std::string, Texture> textures;
	std::vector<Model> models;
	std::unordered_map<std::string, Handle<Model>> name_to_model;

	std::vector<DrawCommand> draw_commands;

public:
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 camera_pos;

	TextDraw text_draw;
	DebugDraw debug_draw;

	// Render passes
	RenderPass *default_pass;
	UnlitPass unlit_pass;
	PBRPass pbr_pass;
	SkyboxPass skybox_pass;

	// Framebuffers
	Framebuffer render_framebuffer;

	glm::vec2 render_extent;

	static RenderManager *get();

	void startup();
	void shutdown();
	void draw();

	void resize_framebuffer(uint32_t width, uint32_t height);

	Handle<Model> load_model(const char *path);
	void load_texture(const char *path);

	Model &get_model(Handle<Model> handle);

	void queue_draw(ModelInstance *model_instance, Transform *transform);
};

#endif // SILENCE_RENDER_MANAGER_H