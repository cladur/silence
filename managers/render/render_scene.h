#ifndef SILENCE_RENDER_SCENE_H
#define SILENCE_RENDER_SCENE_H

#include "common/framebuffer.h"
#include "common/render_pass.h"
#include "components/camera_component.h"
#include "debug/debug_draw.h"
#include "render/common/framebuffer.h"
#include "render/common/render_pass.h"
#include "render/transparent_elements/ui/sprite_draw.h"
#include "transparent_elements/text/text_draw.h"
#include "transparent_elements/transparent_object.h"

#include "debug_camera/debug_camera.h"

struct RenderScene {
	glm::mat4 projection;
	glm::mat4 view;
	glm::vec3 camera_pos;

	DebugCamera debug_camera;
	Transform camera_transform;
	Camera camera_params;
	Frustum frustum;
	float aspect_ratio;

	RenderPass *default_pass;
	GBufferPass g_buffer_pass;
	PBRPass pbr_pass;
	SkyboxPass skybox_pass;
	TransparentPass transparent_pass;
	AOPass ssao_pass;
	AOBlurPass ssao_blur_pass;
	CombinationPass combination_pass;
	BloomPass bloom_pass;

	Framebuffer render_framebuffer;
	Framebuffer final_framebuffer;
	GBuffer g_buffer;
	PBRBuffer pbr_buffer;
	SSAOBuffer ssao_buffer;
	CombinationBuffer combination_buffer;
	BloomBuffer bloom_buffer;
	SkyboxBuffer skybox_buffer;
	glm::vec2 render_extent;

	DebugDraw debug_draw;

	std::vector<TransparentObject> transparent_objects;

	std::vector<DrawCommand> draw_commands;

	bool draw_skybox = false;

	void startup();
	void draw_viewport(bool right_side = false);
	void draw();
	void resize_framebuffer(uint32_t width, uint32_t height);

	void queue_draw(ModelInstance *model_instance, Transform *transform);
	void queue_skinned_draw(SkinnedModelInstance *model_instance, Transform *transform);
};

#endif //SILENCE_RENDER_SCENE_H