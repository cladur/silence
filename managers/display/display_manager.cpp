#include "display_manager.h"
#include <GLFW/glfw3.h>

extern InputManager *input_manager;

DisplayManager *DisplayManager::get() {
	static DisplayManager display_manager;
	return &display_manager;
}

DisplayManager::Status DisplayManager::startup() {
	if (!glfwInit()) {
		return Status::FailedToInitializeGlfw;
	}

	// No need to create context since we're using Vulkan, not OpenGL(ES).

#ifdef USE_OPENGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1280, 720, "Silence Game", nullptr, nullptr);

#ifdef USE_OPENGL
	glfwMakeContextCurrent(window);
#endif

	if (!window) {
		glfwTerminate();
		return Status::FailedToCreateWindow;
	}

	if (!glfwVulkanSupported()) {
		glfwTerminate();
		return Status::VulkanNotSupported;
	}
	return Status::Ok;
}

void DisplayManager::shutdown() {
}

void DisplayManager::capture_mouse(bool capture) const {
	glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

int DisplayManager::get_refresh_rate() const {
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	return mode->refreshRate;
}

#ifndef USE_OPENGL
VkSurfaceKHR DisplayManager::create_surface(VkInstance &instance) const {
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (err) {
		assert(false);
	}
	return surface;
}
#endif

void DisplayManager::poll_events() {
	ZoneScopedN("DisplayManager::poll_events");
	glfwPollEvents();
}

bool DisplayManager::window_should_close() const {
	assert(window != nullptr);
	return glfwWindowShouldClose(window);
}

glm::vec2 DisplayManager::get_framebuffer_size() const {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return { width, height };
}