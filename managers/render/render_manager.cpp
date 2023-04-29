#include "render_manager.h"

#include "components/transform_component.h"
#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "managers/render/common/material.h"
#include <glad/glad.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string shader_path(std::string_view path) {
	return std::string(SHADER_PATH) + std::string(path);
}

RenderManager &RenderManager::get() {
	static RenderManager render_manager;
	return render_manager;
}

void RenderManager::startup() {
	// Initialize OpenGL loader
	bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (err) {
		SPDLOG_ERROR("Failed to initialize OpenGL loader!");
		assert(false);
	}
	SPDLOG_INFO("Successfully initialized OpenGL loader!");

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	DisplayManager &display_manager = DisplayManager::get();

	ImGui_ImplGlfw_InitForOpenGL(display_manager.window, true);
	const char *glsl_version = "#version 410";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup style
	ImGui::StyleColorsDark();
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	text_draw.startup();
	debug_draw.startup();

	unlit_pass.startup();
	pbr_pass.startup();
	skybox_pass.startup();
	default_pass = &pbr_pass;

	glm::vec2 window_extent = display_manager.get_framebuffer_size();
	render_framebuffer.startup(window_extent.x, window_extent.y);
	render_extent = window_extent;
}

void RenderManager::shutdown() {
}

void RenderManager::resize_framebuffer(uint32_t width, uint32_t height) {
	glViewport(0, 0, (int)width, (int)height);
	render_framebuffer.resize(width, height);

	render_extent = glm::vec2(width, height);
}

void RenderManager::draw(Camera &camera) {
	DisplayManager &display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager.get_framebuffer_size();

	//	// Resize the viewport if the window was resized
	//	if (display_manager->is_window_resizable && display_manager->was_window_resized()) {
	//		glViewport(0, 0, (int)window_extent.x, (int)window_extent.y);
	//		render_framebuffer.resize(window_extent.x, window_extent.y);
	//	}

	// Draw grid
	for (int i = -10; i <= 10; i++) {
		debug_draw::draw_line(glm::vec3(i, 0, -10), glm::vec3(i, 0, 10), glm::vec4(0.5, 0.5, 0.5, 1));
		debug_draw::draw_line(glm::vec3(-10, 0, i), glm::vec3(10, 0, i), glm::vec4(0.5, 0.5, 0.5, 1));
	}

	render_framebuffer.bind();

	// Clear the screen
	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection =
			glm::perspective(glm::radians(70.0f), render_extent.x / render_extent.y, 0.1f, cvar_draw_distance.get());
	view = camera.get_view_matrix();
	camera_pos = camera.get_position();

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	unlit_pass.draw();
	pbr_pass.draw();

	glDepthFunc(GL_LEQUAL);
	skybox_pass.draw();
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	text_draw.draw();
	glDisable(GL_BLEND);

	debug_draw.draw();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// IMGUI
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	// Update and Render additional Platform Windows
	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		GLFWwindow *backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}

	glfwSwapBuffers(display_manager.window);
}

Handle<Model> RenderManager::load_model(const char *path) {
	if (name_to_model.find(path) != name_to_model.end()) {
		return name_to_model[path];
	}

	Model model = {};
	model.load_from_asset(asset_path(path).c_str());

	models.push_back(model);
	Handle<Model> handle = {};
	handle.id = models.size() - 1;

	name_to_model[path] = handle;
	return handle;
}

void RenderManager::load_texture(const char *path) {
	if (textures.find(path) != textures.end()) {
		return;
	}
	Texture texture = {};
	texture.load_from_asset(asset_path(path));
	textures[path] = texture;
}

Model &RenderManager::get_model(Handle<Model> handle) {
	return models[handle.id];
}

void RenderManager::queue_draw(ModelInstance *model_instance, Transform *transform) {
	DrawCommand draw_command = {};
	draw_command.model_instance = model_instance;
	draw_command.transform = transform;

	switch (model_instance->material_type) {
		case MaterialType::Default: {
			default_pass->draw_commands.push_back(draw_command);
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
