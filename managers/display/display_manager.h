#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

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

	static DisplayManager *get();

	Status startup();
	void shutdown();

	void capture_mouse(bool capture) const;

	[[nodiscard]] int get_refresh_rate() const;

	VkSurfaceKHR create_surface(VkInstance &instance) const;

	[[nodiscard]] glm::vec2 get_framebuffer_size() const;
	void poll_events();
	[[nodiscard]] bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
