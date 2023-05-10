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

Handle<Model> RenderManager::load_model(const char *path) {
	std::string name = path;
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	if (name_to_model.find(name) != name_to_model.end()) {
		return name_to_model[name];
	}

	Model model = {};
	model.load_from_asset(path);

	models.push_back(model);
	Handle<Model> handle = {};
	handle.id = models.size() - 1;

	name_to_model[name] = handle;
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

Handle<Animation> RenderManager::load_animation(const char *path) {
	if (name_to_animation.find(path) != name_to_animation.end()) {
		return name_to_animation[path];
	}

	Animation animation = {};
	animation.load_from_asset(asset_path(path).c_str());

	animations.push_back(animation);
	Handle<Animation> handle = {};
	handle.id = animations.size() - 1;

	name_to_animation[path] = handle;
	return handle;
}

Animation &RenderManager::get_animation(Handle<Animation> handle) {
	return animations[handle.id];
}

Handle<Animation> RenderManager::get_animation_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_animation[name];
}

std::vector<Animation> &RenderManager::get_animations() {
	return animations;
}

SkinnedModel &RenderManager::get_skinned_model(Handle<SkinnedModel> handle) {
	return skinned_models[handle.id];
}

Handle<SkinnedModel> RenderManager::get_skinned_model_handle(std::string name) {
	bool found_asset_path = name.find(ASSET_PATH) != std::string::npos;
	if (found_asset_path) {
		name = remove_asset_path(name);
	}
	return name_to_skinned_model[name];
}

std::vector<SkinnedModel> &RenderManager::get_skinned_models() {
	return skinned_models;
}

Handle<SkinnedModel> RenderManager::load_skinned_model(const char *path) {
	if (name_to_skinned_model.find(path) != name_to_skinned_model.end()) {
		return name_to_skinned_model[path];
	}

	SkinnedModel model = {};
	model.load_from_asset(asset_path(path).c_str());

	skinned_models.push_back(model);
	Handle<SkinnedModel> handle = {};
	handle.id = skinned_models.size() - 1;

	name_to_skinned_model[path] = handle;
	return handle;
}
std::string RenderManager::get_model_name(Handle<Model> handle) {
	for (auto &pair : name_to_model) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
}

std::string RenderManager::get_skinned_model_name(Handle<SkinnedModel> handle) {
	for (auto &pair : name_to_skinned_model) {
		if (pair.second.id == handle.id) {
			return pair.first;
		}
	}
	return "";
}