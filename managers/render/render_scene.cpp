#include "render_scene.h"

#include "cvars/cvars.h"
#include "game/menu_test.h"
#include "render_manager.h"
#include <glad/glad.h>

AutoCVarFloat cvar_fov = AutoCVarFloat("render.fov", "field of view", 70.0f);
AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

void RenderScene::startup() {
	unlit_pass.startup();
	g_buffer_pass.startup();
	pbr_pass.startup();
	skybox_pass.startup();
	default_pass = &pbr_pass;

	// Size of the viewport doesn't matter here, it will be resized either way
	render_extent = glm::vec2(100, 100);
	render_framebuffer.startup(render_extent.x, render_extent.y);
	g_buffer.startup(render_extent.x, render_extent.y);

	text_draw.current_scene = this;
	sprite_draw.current_scene = this;
	ui_draw.current_scene = this;

	debug_draw.startup();
	transparent_pass.startup();
}

void RenderScene::draw() {
	debug_draw.projection = projection;
	debug_draw.view = view;

	// this just pushes all the elements to be drawn with transparent_pass.draw()
	// no actual drawing happens here
	//	sprite_draw.current_scene = this;
	//	text_draw.current_scene = this;
	//	ui_draw.current_scene = this;
	//	ui_draw.draw();

	g_buffer.bind();
	glViewport(0, 0, (int)render_extent.x, (int)render_extent.y);

	// Clear the screen
	glad_glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection = glm::perspective(
			glm::radians(cvar_fov.get()), render_extent.x / render_extent.y, 0.1f, cvar_draw_distance.get());
	view = camera.get_view_matrix();
	camera_pos = camera.get_position();

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// unlit_pass.draw(*this);
	g_buffer_pass.draw(*this);

	render_framebuffer.bind();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pbr_pass.draw(*this);

	// 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
	// ----------------------------------------------------------------------------------
	glBindFramebuffer(GL_READ_FRAMEBUFFER, g_buffer.framebuffer_id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_framebuffer.framebuffer_id); // write to default framebuffer
	// blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and
	// default framebuffer have to match. the internal formats are implementation defined. This works on all of my
	// systems, but if it doesn't on yours you'll likely have to write to the depth buffer in another shader stage (or
	// somehow see to match the default framebuffer's internal format with the FBO's internal format).
	glBlitFramebuffer(0, 0, render_extent.x, render_extent.y, 0, 0, render_extent.x, render_extent.y,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	render_framebuffer.bind();

	if (draw_skybox) {
		glDepthFunc(GL_LEQUAL);
		skybox_pass.draw(*this);
		glDepthFunc(GL_LESS);
	}

	// Draw grid
	for (int i = -10; i <= 10; i++) {
		glm::vec4 color = glm::vec4(0.5, 0.5, 0.5, 1);
		if (i == 0) {
			color = glm::vec4(0.75, 0.75, 0.75, 1);
		}
		debug_draw.draw_line(glm::vec3(i, 0, -10), glm::vec3(i, 0, 10), color);
		debug_draw.draw_line(glm::vec3(-10, 0, i), glm::vec3(10, 0, i), color);
	}

	debug_draw.draw();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// ui needs to go last, later to be a different render target
	//sprite_draw.draw();
	// transparent_pass.draw(*this);
	glDisable(GL_BLEND);
}

void RenderScene::resize_framebuffer(uint32_t width, uint32_t height) {
	render_framebuffer.resize(width, height);
	g_buffer.resize(width, height);

	render_extent = glm::vec2(width, height);
}

void RenderScene::queue_draw(ModelInstance *model_instance, Transform *transform) {
	DrawCommand draw_command = {};
	draw_command.model_instance = model_instance;
	draw_command.transform = transform;

	switch (model_instance->material_type) {
		case MaterialType::Default: {
			unlit_pass.draw_commands.push_back(draw_command);
			break;
		}
		case MaterialType::Unlit: {
			g_buffer_pass.draw_commands.push_back(draw_command);
			break;
		}
		case MaterialType::PBR: {
			g_buffer_pass.draw_commands.push_back(draw_command);
			break;
		}
	}
}