#include "opengl_manager.h"

#include "display/display_manager.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#define IMGUI_DEFINE_MATH_OPERATORS

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

	mesh.load_from_asset("resources/assets_export/Duck_GLTF/MESH_0_LOD3spShape.mesh");
	shader.load_from_files("resources/shaders/unlit.vert", "resources/shaders/unlit.frag");
	texture.load_from_file("resources/assets_export/DuckCM.ktx2");
}

void OpenglManager::shutdown() {
}

void OpenglManager::draw(Camera &camera) {
	DisplayManager *display_manager = DisplayManager::get();
	glm::vec2 window_extent = display_manager->get_framebuffer_size();

	// Clear the screen
	glad_glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw meshes
	shader.use();
	shader.set_mat4("view", camera.get_view_matrix());
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), window_extent.x / window_extent.y, 0.1f, 100.0f);
	shader.set_mat4("projection", projection);
	shader.set_mat4("model", glm::mat4(1.0f));
	shader.set_vec3("camPos", camera.get_position());
	shader.set_int("albedo_map", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture.id);
	mesh.draw(shader);

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

void OpenglManager::update_transform(uint32_t object_id, glm::mat4 const &transform) {
	// TODO
}
