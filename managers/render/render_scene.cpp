#include "render_scene.h"

#include "cvars/cvars.h"
#include "render_manager.h"

AutoCVarFloat cvar_fov = AutoCVarFloat("render.fov", "field of view", 70.0f);
AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

void RenderScene::startup() {
	unlit_pass.startup();
	pbr_pass.startup();
	skybox_pass.startup();
	default_pass = &pbr_pass;

	// Size of the viewport doesn't matter here, it will be resized either way
	render_extent = glm::vec2(100, 100);
	render_framebuffer.startup(render_extent.x, render_extent.y);

	text_draw.startup();
	debug_draw.startup();
}

void RenderScene::draw() {
	text_draw.render_extent = render_extent;
	debug_draw.projection = projection;
	debug_draw.view = view;

	render_framebuffer.bind();
	glViewport(0, 0, (int)render_extent.x, (int)render_extent.y);

	// Draw grid
	for (int i = -10; i <= 10; i++) {
		debug_draw.draw_line(glm::vec3(i, 0, -10), glm::vec3(i, 0, 10), glm::vec4(0.5, 0.5, 0.5, 1));
		debug_draw.draw_line(glm::vec3(-10, 0, i), glm::vec3(10, 0, i), glm::vec4(0.5, 0.5, 0.5, 1));
	}

	// Clear the screen
	glad_glClearColor(0.275f, 0.275f, 0.275f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection = glm::perspective(glm::radians(cvar_fov.get()), render_extent.x / render_extent.y, 0.1f, cvar_draw_distance.get());
	view = camera.get_view_matrix();
	camera_pos = camera.get_position();

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	unlit_pass.draw(*this);
	pbr_pass.draw(*this);

	if (draw_skybox) {
		glDepthFunc(GL_LEQUAL);
		skybox_pass.draw(*this);
		glDepthFunc(GL_LESS);
	}

	debug_draw.draw();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	text_draw.draw();
	glDisable(GL_BLEND);
}

void RenderScene::resize_framebuffer(uint32_t width, uint32_t height) {
	render_framebuffer.resize(width, height);

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
			unlit_pass.draw_commands.push_back(draw_command);
			break;
		}
		case MaterialType::PBR: {
			pbr_pass.draw_commands.push_back(draw_command);
			break;
		}
	}
}