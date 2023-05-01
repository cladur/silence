#include "render_manager.h"

#include "components/transform_component.h"
#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "managers/render/common/material.h"
#include "render/render_scene.h"
#include <glad/glad.h>
#define IMGUI_DEFINE_MATH_OPERATORS

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string remove_asset_path(std::string_view path) {
	std::size_t asset_path_pos = path.find(ASSET_PATH);

	if (asset_path_pos != std::string::npos) {
		return std::string(path).substr(asset_path_pos + std::strlen(ASSET_PATH));
	}

	return std::string(path);
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
}

void RenderManager::shutdown() {
}

uint32_t RenderManager::create_render_scene() {
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
std::vector<Model> &RenderManager::get_models() {
	return models;
}
Handle<Model> RenderManager::get_model_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_model[name];
}
