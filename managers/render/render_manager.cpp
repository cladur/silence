#include "render_manager.h"

#include "common/material.h"
#include "components/transform_component.h"
#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "render/render_scene.h"
#include <glad/glad.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

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
}

void RenderManager::shutdown() {
}

uint32_t RenderManager::create_render_scene() {
	static int scene_count = 0;
	RenderScene render_scene;
	render_scene.startup();
	render_scenes.push_back(render_scene);
	return render_scenes.size() - 1;
}

void RenderManager::draw() {
	DisplayManager &display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager.get_framebuffer_size();

	//	// Resize the viewport if the window was resized
	//	if (display_manager->is_window_resizable && display_manager->was_window_resized()) {
	//		glViewport(0, 0, (int)window_extent.x, (int)window_extent.y);
	//		render_framebuffer.resize(window_extent.x, window_extent.y);
	//	}

	for (RenderScene &scene : render_scenes) {
		scene.draw();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// blit scene's framebuffer to the default framebuffer
	if (displayed_scene != -1) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, render_scenes[displayed_scene].render_framebuffer.framebuffer_id);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, window_extent.x, window_extent.y, 0, 0, window_extent.x, window_extent.y,
				GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

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
