#include "opengl_manager.h"

#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define IMGUI_DEFINE_MATH_OPERATORS

#define ASSET_PATH "resources/assets_export/"
#define SHADER_PATH "resources/shaders/"

std::string asset_path(std::string_view path) {
	return std::string(ASSET_PATH) + std::string(path);
}

std::string shader_path(std::string_view path) {
	return std::string(SHADER_PATH) + std::string(path);
}

OpenglManager *OpenglManager::get() {
	static OpenglManager opengl_manager;
	return &opengl_manager;
}

void OpenglManager::startup() {
	// Initialize OpenGL loader
	bool err = !gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	if (err) {
		SPDLOG_ERROR("Failed to initialize OpenGL loader!");
		assert(false);
	}
	SPDLOG_INFO("Successfully initialized OpenGL loader!");

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

	shader.load_from_files("resources/shaders/unlit.vert", "resources/shaders/unlit.frag");
	model.load_from_asset(asset_path("DamagedHelmet/DamagedHelmet.pfb").c_str());
}

void OpenglManager::shutdown() {
}

void OpenglManager::draw(Camera &camera) {
	DisplayManager *display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager->get_framebuffer_size();

	// Clear the screen
	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Enable depth testing
	glEnable(GL_DEPTH_TEST);

	// Draw meshes
	shader.use();
	shader.set_mat4("view", camera.get_view_matrix());
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), window_extent.x / window_extent.y, 0.1f, 100.0f);
	shader.set_mat4("projection", projection);
	shader.set_mat4("model", glm::mat4(1.0f));
	shader.set_vec3("camPos", camera.get_position());
	shader.set_int("albedo_map", 0);
	shader.set_int("ao_map", 1);
	shader.set_int("normal_map", 2);
	shader.set_int("metallic_roughness_map", 3);
	shader.set_int("emissive_map", 4);
	model.draw(shader);

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

void OpenglManager::load_mesh(const char *path) {
	if (meshes.find(path) != meshes.end()) {
		return;
	}
	Mesh mesh = {};
	mesh.load_from_asset(asset_path(path).c_str());
	meshes[path] = mesh;
}

void OpenglManager::load_model(const char *path) {
	if (models.find(path) != models.end()) {
		return;
	}
	Model model = {};
	model.load_from_asset(asset_path(path).c_str());
	models[path] = model;
}

void OpenglManager::load_texture(const char *path) {
	if (textures.find(path) != textures.end()) {
		return;
	}
	Texture texture = {};
	texture.load_from_asset(asset_path(path).c_str());
	textures[path] = texture;
}

void OpenglManager::update_transform(uint32_t object_id, glm::mat4 const &transform) {
	// TODO
}
