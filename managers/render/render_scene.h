#ifndef SILENCE_RENDER_SCENE_H
#define SILENCE_RENDER_SCENE_H

#include "common/framebuffer.h"
#include "common/render_pass.h"
#include "debug/debug_draw.h"
#include "text/text_draw.h"

#include "camera/camera.h"

struct RenderScene {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 camera_pos;

	Camera camera;

	RenderPass *default_pass;
	UnlitPass unlit_pass;
	PBRPass pbr_pass;
	SkyboxPass skybox_pass;

	Framebuffer render_framebuffer;
	glm::vec2 render_extent;

	TextDraw text_draw;
	DebugDraw debug_draw;

	std::vector<DrawCommand> draw_commands;

	bool draw_skybox = false;

	void startup();
	void draw();
	void resize_framebuffer(uint32_t width, uint32_t height);

	void queue_draw(ModelInstance *model_instance, Transform *transform);
};

#endif //SILENCE_RENDER_SCENE_H