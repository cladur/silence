#ifndef SILENCE_DISPLAY_MANAGER_H
#define SILENCE_DISPLAY_MANAGER_H

#define GLFW_INCLUDE_VULKAN
#include "../core/input/input_key.h"
#include "../core/input/input_manager.h"
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <utility>

class DisplayManager {
public:
	enum class Status {
		Ok,
		FailedToInitializeGlfw,
		FailedToCreateWindow,
		VulkanNotSupported,
	};

	GLFWwindow *window;

	Status startup();
	void shutdown();
	
	VkSurfaceKHR create_surface(VkInstance &instance) const;

	[[nodiscard]] std::pair<int, int> get_framebuffer_size() const;
	void poll_events();
	[[nodiscard]] bool window_should_close() const;
};

#endif //SILENCE_DISPLAY_MANAGER_H
