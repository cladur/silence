#ifndef SILENCE_RENDER_SCENE_H
#define SILENCE_RENDER_SCENE_H

#include "common/framebuffer.h"
#include "common/render_pass.h"
#include "debug/debug_draw.h"
#include "render/common/render_pass.h"
#include "render/transparent_elements/ui/sprite_draw.h"
#include "transparent_elements/text/text_draw.h"
#include "transparent_elements/transparent_object.h"
//#include <game/menu_test.h>

#include "camera/camera.h"
#include "render/transparent_elements/ui_draw.h"

struct RenderScene {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 camera_pos;

	Camera camera;

	RenderPass *default_pass;
	SkinnedPassUnlit skinned_unlit_pass;
	GBufferPass g_buffer_pass;
	PBRPass pbr_pass;
	SkyboxPass skybox_pass;
	TransparentPass transparent_pass;

	Framebuffer render_framebuffer;
	GBuffer g_buffer;
	glm::vec2 render_extent;

	TextDraw text_draw;
	DebugDraw debug_draw;
	SpriteDraw sprite_draw;
	UIDraw ui_draw;
	std::vector<TransparentObject> transparent_objects;

	std::vector<DrawCommand> draw_commands;

	bool draw_skybox = false;

	void startup();
	void draw();
	void resize_framebuffer(uint32_t width, uint32_t height);

	void queue_draw(ModelInstance *model_instance, Transform *transform);
	void queue_skinned_draw(SkinnedModelInstance *model_instance, Transform *transform);
};

#endif //SILENCE_RENDER_SCENE_H