#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#include <glad/glad.h>

#ifdef USE_OPENGL
#define GLFW_INCLUDE_NONE
#else
#define GLFW_INCLUDE_VULKAN
#include <vulkan/vulkan.h>
#endif

#include <GLFW/glfw3.h>

#include "managers/input/input_key.h"
#include "managers/input/input_manager.h"

class DisplayManager {
public:
	enum class Status {
		Ok,
		FailedToInitializeGlfw,
		FailedToCreateWindow,
		VulkanNotSupported,
	};

	GLFWwindow *window;

	bool is_window_resizable;

	static DisplayManager *get();

	Status startup(bool resizable = false);
	void shutdown();

	void capture_mouse(bool capture) const;

	[[nodiscard]] int get_refresh_rate() const;

#ifndef USE_OPENGL
	VkSurfaceKHR create_surface(VkInstance &instance) const;
#endif

	[[nodiscard]] glm::vec2 get_framebuffer_size() const;
	void poll_events();
	[[nodiscard]] bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
