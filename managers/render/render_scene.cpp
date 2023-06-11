#include "render_scene.h"

#include "cvars/cvars.h"
#include "display/display_manager.h"
#include "game/menu_test.h"
#include "render/transparent_elements/ui_manager.h"
#include "render_manager.h"
#include <glad/glad.h>
#include <render/transparent_elements/particle_manager.h>

AutoCVarFloat cvar_draw_distance_near("render.draw_distance.near", "Near distance cull", 0.001f);
AutoCVarFloat cvar_draw_distance_far("render.draw_distance.far", "Far distance cull", 5000);
AutoCVarInt cvar_frustum_freeze("render.frustum.freeze", "Freeze frustum", 0, CVarFlags::EditCheckbox);
AutoCVarInt cvar_frustum_force_scene_camera("render.frustum.force_scene_camera",
		"Force frustum culling using scene camera, even if we're currently using debug camera", 0,
		CVarFlags::EditCheckbox);
AutoCVarInt cvar_debug_camera_use("debug_camera.use", "Use debug camera", 1, CVarFlags::EditCheckbox);

// SSAO Params
AutoCVarInt cvar_ssao("render.ssao", "Use SSAO", 1, CVarFlags::EditCheckbox);
AutoCVarFloat cvar_ssao_radius("render.ssao.radius", "SSAO radius", 0.5f);
AutoCVarFloat cvar_ssao_bias("render.ssao.bias", "SSAO bias", 0.04f);
AutoCVarInt cvar_ao_blur("render.ssao_blur", "Should SSAO be blurred", 1, CVarFlags::EditCheckbox);

AutoCVarInt cvar_splitscreen("render.splitscreen", "Splitscreen", 0, CVarFlags::EditCheckbox);

void RenderScene::startup() {
	g_buffer_pass.startup();
	pbr_pass.startup();
	light_pass.startup();
	skybox_pass.startup();
	ssao_pass.startup();
	ssao_blur_pass.startup();
	combination_pass.startup();
	default_pass = &pbr_pass;
	bloom_pass.startup();
	shadow_pass.startup();
	mouse_pick_pass.startup();
	particle_pass.startup();
	highlight_pass.startup();
	decal_pass.startup();

	// Size of the viewport doesn't matter here, it will be resized either way
	render_extent = glm::vec2(100, 100);
	final_framebuffer.startup(render_extent.x, render_extent.y);
	render_framebuffer.startup(render_extent.x, render_extent.y);
	g_buffer.startup(render_extent.x, render_extent.y);
	ssao_buffer.startup(render_extent.x, render_extent.y);
	pbr_buffer.startup(render_extent.x, render_extent.y);
	bloom_buffer.startup(render_extent.x, render_extent.y, 5);
	shadow_buffer.startup(2048, 2048, 0.0001f, 25.0f);
	combination_buffer.startup(render_extent.x, render_extent.y);
	skybox_buffer.startup(render_extent.x, render_extent.y);
	mouse_pick_framebuffer.startup(render_extent.x, render_extent.y);
	particle_buffer.startup(render_extent.x, render_extent.y);
	highlight_buffer.startup(render_extent.x, render_extent.y);

	debug_draw.startup();
	transparent_pass.startup();

	debug_camera = DebugCamera(glm::vec3(-4.0f, 2.6f, -4.0f));
	debug_camera.yaw = 45.0f;
	debug_camera.pitch = -20.0f;
	debug_camera.update_camera_vectors();
	UIManager::get().set_render_scene(this);
}

void RenderScene::draw_viewport(bool right_side) {
	ZoneScopedN("RenderScene::draw_viewport");
	glViewport(0, 0, (int)shadow_buffer.shadow_width, (int)shadow_buffer.shadow_height);
	shadow_buffer.bind();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	shadow_pass.draw(*this);

	glDepthMask(GL_TRUE);
	glViewport(0, 0, (int)render_extent.x, (int)render_extent.y);
	g_buffer.bind();

	// Clear the screen
	glad_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	aspect_ratio = render_extent.x / render_extent.y;

	Camera &camera_params = right_side ? right_camera_params : left_camera_params;
	Transform &camera_transform = right_side ? right_camera_transform : left_camera_transform;

	if (!cvar_frustum_freeze.get()) {
		if (cvar_debug_camera_use.get() && !cvar_frustum_force_scene_camera.get()) {
			frustum.create_frustum_from_camera(debug_camera, aspect_ratio);
		} else {
			frustum.create_frustum_from_camera(camera_params, camera_transform, aspect_ratio);
		}
	}

	if (cvar_debug_camera_use.get()) {
		projection = glm::perspective(glm::radians(*CVarSystem::get()->get_float_cvar("debug_camera.fov")),
				aspect_ratio, cvar_draw_distance_near.get(), cvar_draw_distance_far.get());
		view = debug_camera.get_view_matrix();
		camera_pos = debug_camera.get_position();
	} else {
		projection = glm::perspective(glm::radians(camera_params.fov), aspect_ratio, cvar_draw_distance_near.get(),
				cvar_draw_distance_far.get());
		view = glm::inverse(camera_transform.get_global_model_matrix());
		camera_pos = camera_transform.get_global_position();
	}

	g_buffer_pass.draw(*this);

	// HIGHLIGHT PASS
	highlight_buffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	// all the see-through highlights
	highlight_pass.draw_xray(*this, right_side);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, highlight_buffer.framebuffer_id); // write to default framebuffer
	glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glDepthFunc(GL_LEQUAL);

	// all the normal highlights
	highlight_pass.draw_normal(*this, right_side);

	glDepthMask(GL_TRUE);

	pbr_buffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	pbr_pass.draw(*this);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	light_pass.draw(*this);

	glDisable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	ssao_buffer.bind();
	glad_glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (cvar_ssao.get()) {
		ssao_pass.material.radius = cvar_ssao_radius.get();
		ssao_pass.material.bias = cvar_ssao_bias.get();
		ssao_pass.draw(*this);
	}

	// render_framebuffer.bind();
	// ssao_blur_pass.material.should_blur = cvar_ao_blur.get();
	// ssao_blur_pass.draw(*this);
	// glBindFramebuffer(GL_READ_FRAMEBUFFER, ssao_buffer.framebuffer_id);

	// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
	// ----------------------------------------------------------------------------------
	render_framebuffer.bind();
	glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_framebuffer.framebuffer_id); // write to default framebuffer
	// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and
	// default framebuffer have to match. the internal formats are implementation defined. This works on all of my
	// systems, but if it doesn't on yours you'll likely have to write to the depth buffer in another shader stage (or
	// somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	skybox_buffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDepthFunc(GL_LEQUAL);
	skybox_pass.draw(*this);
	glDepthFunc(GL_LESS);

	particle_buffer.bind();
	// need to clear alpha channel
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, particle_buffer.framebuffer_id);
	glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);

	glDepthMask(GL_FALSE);
	particle_pass.draw(*this, right_side);

	glDepthMask(GL_TRUE);

	combination_buffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);

	combination_pass.draw(*this);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, combination_buffer.framebuffer_id); // write to default framebuffer
	// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and
	// default framebuffer have to match. the internal formats are implementation defined. This works on all of my
	// systems, but if it doesn't on yours you'll likely have to write to the depth buffer in another shader stage (or
	// somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glDepthMask(GL_TRUE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	transparent_pass.draw_worldspace(*this);
	glDisable(GL_BLEND);
	glDepthMask(GL_FALSE);

	bloom_buffer.bind();
	bloom_pass.draw(*this);

	debug_draw.projection = projection;
	debug_draw.view = view;

	glDepthMask(GL_TRUE);
	// Draw grid
	for (int i = -10; i <= 10; i++) {
		glm::vec4 color = glm::vec4(0.5, 0.5, 0.5, 1);
		if (i == 0) {
			color = glm::vec4(0.75, 0.75, 0.75, 1);
		}
		//debug_draw.draw_line(glm::vec3(i, 0, -10), glm::vec3(i, 0, 10), color);
		//debug_draw.draw_line(glm::vec3(-10, 0, i), glm::vec3(10, 0, i), color);
	}

	debug_draw.draw();

	mouse_pick_framebuffer.bind();
	glad_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mouse_pick_pass.draw(*this);
	debug_draw.draw_mouse_pick();
}

void RenderScene::draw() {
	ZoneScopedN("RenderScene::draw");
	static bool was_splitscreen = cvar_splitscreen.get();
	static bool was_debug_camera = cvar_debug_camera_use.get();
	if (was_splitscreen != cvar_splitscreen.get() || was_debug_camera != cvar_debug_camera_use.get()) {
		was_splitscreen = cvar_splitscreen.get();
		was_debug_camera = cvar_debug_camera_use.get();
		glm::vec2 window_extent = DisplayManager::get().get_framebuffer_size();
		resize_framebuffer(window_extent.x, window_extent.y);
	}

	camera_near_far = glm::vec2(cvar_draw_distance_near.get(), cvar_draw_distance_far.get());

	UIManager::get().set_render_scene(this);
	UIManager::get().draw();

	transparent_pass.sort_objects(*this);

	highlight_pass.sort_highlights(*this);

	if (cvar_splitscreen.get() && !cvar_debug_camera_use.get()) {
		draw_viewport(false); // agent
		{
			ZoneScopedN("RenderScene::draw");
			glBindFramebuffer(GL_READ_FRAMEBUFFER, render_framebuffer.framebuffer_id);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_framebuffer.framebuffer_id);
			glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		draw_viewport(true); // hacker
		{
			ZoneScopedN("RenderScene::blit");
			glBindFramebuffer(GL_READ_FRAMEBUFFER, render_framebuffer.framebuffer_id);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_framebuffer.framebuffer_id);
			glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, render_extent.x, 0, 2 * render_extent.x,
					render_extent.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	} else {
		int *controlling_agent = CVarSystem::get()->get_int_cvar("game.controlling_agent");
		bool right_side = false;
		if (controlling_agent != nullptr) {
			right_side = !*controlling_agent;
		}
		draw_viewport(right_side);

		{
			ZoneScopedN("RenderScene::blit");
			glBindFramebuffer(GL_READ_FRAMEBUFFER, render_framebuffer.framebuffer_id);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, final_framebuffer.framebuffer_id);
			glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	}

	highlight_pass.clear();

	glViewport(0, 0, full_render_extent.x, full_render_extent.y);
	final_framebuffer.bind();
	// i assume nothing else that needs depth info will be drawn after UI.
	glClear(GL_DEPTH_BUFFER_BIT);

	glEnable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	transparent_pass.draw_screenspace(*this);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	draw_commands.clear();
#ifdef WIN32
	skinned_draw_commands.clear();
#endif
	light_draw_commands.clear();
	decal_draw_commands.clear();
	debug_draw.vertices.clear();
	debug_draw.mouse_pick_vertices.clear();
}

void RenderScene::resize_framebuffer(uint32_t width, uint32_t height) {
	final_framebuffer.resize(width, height);
	full_render_extent = glm::vec2(width, height);

	if (cvar_splitscreen.get() && !cvar_debug_camera_use.get()) {
		width /= 2;
	}

	render_framebuffer.resize(width, height);
	g_buffer.resize(width, height);
	ssao_buffer.resize(width, height);
	pbr_buffer.resize(width, height);
	bloom_buffer.resize(width, height);
	combination_buffer.resize(width, height);
	skybox_buffer.resize(width, height);
	mouse_pick_framebuffer.resize(width, height);
	particle_buffer.resize(width, height);
	highlight_buffer.resize(width, height);

	render_extent = glm::vec2(width, height);
}

void RenderScene::queue_draw(
		ModelInstance *model_instance, Transform *transform, Entity entity, HighlightData highlight_data) {
	DrawCommand draw_command = {};
	draw_command.model_instance = model_instance;
	draw_command.transform = transform;
	draw_command.entity = entity;
	draw_command.highlight_data = highlight_data;

	draw_commands.push_back(draw_command);
}

void RenderScene::queue_skinned_draw(
		SkinnedModelInstance *model_instance, Transform *transform, Entity entity, HighlightData highlight_data) {
	SkinnedDrawCommand draw_command = {};
	draw_command.model_instance = model_instance;
	draw_command.transform = transform;
	draw_command.entity = entity;
	draw_command.highlight_data = highlight_data;

#ifdef WIN32
	skinned_draw_commands.push_back(draw_command);
#endif
}

void RenderScene::queue_light_draw(Light *light, Transform *transform) {
	LightDrawCommand draw_command = {};
	draw_command.light = light;
	draw_command.transform = transform;

	light_draw_commands.push_back(draw_command);
}

Entity RenderScene::get_entity_at_mouse_position(float x, float y) const {
	Entity entity = 0;
	// Invert y
	y = render_extent.y - y;
	// Get entity id from mouse_pick_framebuffer.texture_id texture
	glBindFramebuffer(GL_FRAMEBUFFER, mouse_pick_framebuffer.framebuffer_id);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &entity);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	return entity;
}

void RenderScene::queue_decal_draw(Decal *decal, Transform *transform) {
	DecalDrawCommand draw_command = {};
	draw_command.decal = decal;
	draw_command.transform = transform;

	decal_draw_commands.push_back(draw_command);
}
