#include "render_manager.h"

#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "opengl/material.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

extern Camera camera;

AutoCVarFloat cvar_draw_distance("render.draw_distance", "Distance cull", 5000);

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string shader_path(std::string_view path) {
	return std::string(SHADER_PATH) + std::string(path);
}

RenderManager *RenderManager::get() {
	static RenderManager opengl_manager;
	return &opengl_manager;
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
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	DisplayManager *display_manager = DisplayManager::get();

	ImGui_ImplGlfw_InitForOpenGL(display_manager->window, true);
	const char *glsl_version = "#version 410";
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Setup style
	ImGui::StyleColorsDark();
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[ImGuiCol_WindowBg].w = 1.0f;

	text_draw.startup();
	debug_draw.startup();
	sprite_draw.startup();
	transparent_draw.startup();

	unlit_pass.startup();
	pbr_pass.startup();
	skybox_pass.startup();
}

void RenderManager::shutdown() {
}

void RenderManager::draw() {
	DisplayManager *display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager->get_framebuffer_size();
	if (display_manager->is_window_resizable) {
		glViewport(0, 0, (int)window_extent.x, (int)window_extent.y);
	}

	// Clear the screen
	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection =
			glm::perspective(glm::radians(70.0f), window_extent.x / window_extent.y, 0.1f, cvar_draw_distance.get());
	view = camera.get_view_matrix();
	camera_pos = camera.get_position();

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	unlit_pass.draw();
	pbr_pass.draw();

	glDepthFunc(GL_LEQUAL);
	skybox_pass.draw();
	glDepthFunc(GL_LESS);

	debug_draw.draw();

	glEnable(GL_BLEND);
	//	glEnable(GL_ALPHA_TEST);
	//	glAlphaFunc(GL_LESS, 0.1);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//text_draw.draw();

	// ui needs to go last, later to be a different render target
	//sprite_draw.draw();
	transparent_draw.draw();
	glDisable(GL_BLEND);

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

	glfwSwapBuffers(display_manager->window);
}

void RenderManager::load_model(const char *path) {
	if (name_to_model.find(path) != name_to_model.end()) {
		return;
	}
	Model model = {};
	model.load_from_asset(asset_path(path).c_str());

	models.push_back(model);
	Handle<Model> handle = {};
	handle.id = models.size() - 1;

	name_to_model[path] = handle;
}

void RenderManager::load_texture(const char *path) {
	if (textures.find(path) != textures.end()) {
		return;
	}
	Texture texture = {};
	texture.load_from_asset(asset_path(path).c_str());
	textures[path] = texture;
}

Model &RenderManager::get_model(Handle<Model> handle) {
	return models[handle.id];
}

ModelInstance &RenderManager::get_model_instance(Handle<ModelInstance> handle) {
	return model_instances[handle.id];
}

Handle<ModelInstance> RenderManager::add_instance(const char *path, MaterialType material_type, bool in_shadow_pass) {
	load_model(path);

	ModelInstance model_instance = {};
	model_instance.transform = glm::mat4(1.0f);

	Handle<ModelInstance> handle = {};
	handle.id = model_instances.size();

	model_instances.push_back(model_instance);

	switch (material_type) {
		case MATERIAL_TYPE_UNLIT: {
			unlit_pass.add_instance(handle);
			break;
		}
		case MATERIAL_TYPE_PBR: {
			pbr_pass.add_instance(handle);
			break;
		}
		default: {
			assert(false);
		}
	}

	return handle;
}

void RenderManager::remove_instance(Handle<ModelInstance> handle) {
	model_instances.erase(model_instances.begin() + handle.id);
	unlit_pass.remove_instance(handle);
}

void RenderManager::update_instance_passes(
		Handle<ModelInstance> handle, MaterialType material_type, bool in_shadow_pass) {
	// TODO
	assert(false);
}

void RenderManager::update_instance_transform(Handle<ModelInstance> handle, glm::mat4 const &transform) {
	get_model_instance(handle).transform = transform;
}
