#include "display_manager.h"
#include <GLFW/glfw3.h>

DisplayManager &DisplayManager::get() {
	static DisplayManager display_manager;
	return display_manager;
}

DisplayManager::Status DisplayManager::startup(const std::string &window_name, bool resizable) {
	if (!glfwInit()) {
		return Status::FailedToInitializeGlfw;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac

	is_window_resizable = resizable;
	glfwWindowHint(GLFW_RESIZABLE, resizable);
	window = glfwCreateWindow(1280, 720, window_name.c_str(), nullptr, nullptr);

	glfwGetWindowPos(window, &default_window_pos[0], &default_window_pos[1]);

	glfwMakeContextCurrent(window);

	glfwSwapInterval(0); // Disable vsync

	if (!window) {
		glfwTerminate();
		return Status::FailedToCreateWindow;
	}

	return Status::Ok;
}

void DisplayManager::shutdown() {
}

void DisplayManager::toggle_fullscreen() const {
	if (glfwGetWindowMonitor(window) == nullptr) {
		// Fullscreen
		GLFWmonitor *monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode *mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	} else {
		// Windowed
		glfwSetWindowMonitor(
				window, nullptr, default_window_pos[0], default_window_pos[1], 1280, 720, get_refresh_rate());
	}
}

void DisplayManager::set_windowed_full_hd() const {
	glfwSetWindowMonitor(window, nullptr, default_window_pos[0], default_window_pos[1], 1920, 1080, get_refresh_rate());
}

void DisplayManager::capture_mouse(bool capture) const {
	glfwSetInputMode(window, GLFW_CURSOR, capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

int DisplayManager::get_refresh_rate() const {
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	return mode->refreshRate;
}

[[nodiscard]] bool DisplayManager::was_window_resized() const {
	static glm::vec2 last_framebuffer_size = get_framebuffer_size();
	glm::vec2 framebuffer_size = get_framebuffer_size();
	if (framebuffer_size != last_framebuffer_size) {
		last_framebuffer_size = framebuffer_size;
		return true;
	}
	return false;
}

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

[[nodiscard]] glm::vec2 DisplayManager::get_window_size() const {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return { width, height };
}
