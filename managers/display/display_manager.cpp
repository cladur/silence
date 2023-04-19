#include "display_manager.h"

#include <cassert>

extern DisplayManager display_manager;
extern InputManager *input_manager;

DisplayManager::Status DisplayManager::startup() {
	if (!glfwInit()) {
		return Status::FailedToInitializeGlfw;
	}

	// No need to create context since we're using Vulkan, not OpenGL(ES).
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(1280, 720, "Silence Game", nullptr, nullptr);
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

VkSurfaceKHR DisplayManager::create_surface(VkInstance &instance) const {
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, nullptr, &surface);
	if (err) {
		assert(false);
	}
	return surface;
}

void DisplayManager::poll_events() {
	ZoneScopedN("DisplayManager::poll_events");
	glfwPollEvents();
}

bool DisplayManager::window_should_close() const {
	assert(window != nullptr);
	return glfwWindowShouldClose(window);
}

std::pair<int, int> DisplayManager::get_framebuffer_size() const {
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	return std::make_pair(width, height);
}